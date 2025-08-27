// Fill out your copyright notice in the Description page of Project Settings.

#include "Online/SteamSessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamSessionMgr, Log, All);

// 키 초기화
const FName USteamSessionManager::SESSION_NAME     = TEXT("GameSession");
const FName USteamSessionManager::KEY_SERVER_NAME  = TEXT("SERVER_NAME");
const FName USteamSessionManager::KEY_MAP_NAME     = TEXT("MAP_NAME");

void USteamSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSteamSessionMgr, Log, TEXT("SteamSessionManager Initialized"));
}

void USteamSessionManager::Deinitialize()
{
    ClearAllDelegates();
    SearchHandle.Reset();
    CachedSummaries.Reset();
    Super::Deinitialize();
    UE_LOG(LogSteamSessionMgr, Log, TEXT("SteamSessionManager Deinitialized"));
}

IOnlineSessionPtr USteamSessionManager::GetSession() const
{
    if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
    {
        return OSS->GetSessionInterface();
    }
    return nullptr;
}

void USteamSessionManager::CreateLobby(int32 MaxPlayers, const FString& ServerName, const FString& MapName)
{
    auto Session = GetSession();
    if (!Session.IsValid())
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("No Session Interface"));
        OnCreateSessionFailed.Broadcast();
        OnCreateSessionFailedBP.Broadcast();
        return;
    }

    if (Session->GetNamedSession(SESSION_NAME))
    {
        UE_LOG(LogSteamSessionMgr, Log, TEXT("Existing session found. Destroying first..."));
        bPendingCreateAfterDestroy = true;
        Pending_MaxPlayers = MaxPlayers;
        Pending_ServerName = ServerName;
        Pending_MapName = MapName;

        RegisterDestroyDelegate();
        Session->DestroySession(SESSION_NAME);
        return;
    }

    StartCreateSessionNow(MaxPlayers, ServerName, MapName);
}

void USteamSessionManager::StartCreateSessionNow(int32 MaxPlayers, const FString& ServerName, const FString& MapName)
{
    auto Session = GetSession();
    if (!Session.IsValid()) return;

    TSharedPtr<FOnlineSessionSettings> Settings = MakeShared<FOnlineSessionSettings>();
    Settings->bIsLANMatch = false;                 // Steam은 LAN=false
    Settings->NumPublicConnections = MaxPlayers;
    Settings->bAllowJoinInProgress = true;
    Settings->bShouldAdvertise = true;
    Settings->bUsesPresence = true;
    Settings->bAllowJoinViaPresence = true;
    Settings->bUseLobbiesIfAvailable = true;

    Settings->Set(KEY_SERVER_NAME, ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings->Set(KEY_MAP_NAME,    MapName,    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    RegisterCreateDelegate();
    const bool bReq = Session->CreateSession(0, SESSION_NAME, *Settings);
    if (!bReq)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("CreateSession request failed immediately"));
        OnCreateSessionFailed.Broadcast();
        OnCreateSessionFailedBP.Broadcast();
    }
}

void USteamSessionManager::FindLobbies(bool bLanQuery, int32 MaxResults)
{
    auto Session = GetSession();
    if (!Session.IsValid()) return;

    SearchHandle = MakeShared<FOnlineSessionSearch>();
    SearchHandle->MaxSearchResults = MaxResults;
    SearchHandle->bIsLanQuery = bLanQuery;
    // 최신 UE에서는 SEARCH_PRESENCE 매크로가 없을 수 있으므로 FName 직접 사용
    SearchHandle->QuerySettings.Set(FName(TEXT("PRESENCE")), true, EOnlineComparisonOp::Equals);

    RegisterFindDelegate();
    const bool bReq = Session->FindSessions(0, SearchHandle.ToSharedRef());
    if (!bReq)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("FindSessions request failed immediately"));
        CachedSummaries.Reset();
        OnFindSessionsCompleted.Broadcast();
        OnFindSessionsCompletedBP.Broadcast();
    }
}

void USteamSessionManager::JoinLobbyByIndex(int32 Index)
{
    auto Session = GetSession();
    if (!Session.IsValid() || !SearchHandle.IsValid())
    {
        OnJoinCompletedBP.Broadcast(EJoinResultBP::UnknownError);
        return;
    }

    if (!SearchHandle->SearchResults.IsValidIndex(Index))
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("Join index out of range: %d"), Index);
        OnJoinCompletedBP.Broadcast(EJoinResultBP::UnknownError);
        return;
    }

    RegisterJoinDelegate();
    const bool bReq = Session->JoinSession(0, SESSION_NAME, SearchHandle->SearchResults[Index]);
    if (!bReq)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("JoinSession request failed immediately"));
        OnJoinCompletedBP.Broadcast(EJoinResultBP::UnknownError);
    }
}

void USteamSessionManager::DestroyLobby()
{
    auto Session = GetSession();
    if (!Session.IsValid())
    {
        OnDestroySessionCompleted.Broadcast();
        OnDestroySessionCompletedBP.Broadcast();
        return;
    }

    RegisterDestroyDelegate();
    const bool bReq = Session->DestroySession(SESSION_NAME);
    if (!bReq)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("DestroySession request failed immediately"));
        OnDestroySessionCompleted.Broadcast();
        OnDestroySessionCompletedBP.Broadcast();
    }
}

// ===== Delegate 등록/해제 =====
void USteamSessionManager::RegisterCreateDelegate()
{
    if (auto Session = GetSession())
    {
        if (!OnCreateCompleteHandle.IsValid())
        {
            OnCreateCompleteHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(
                FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamSessionManager::HandleCreateSession));
        }
    }
}

void USteamSessionManager::RegisterFindDelegate()
{
    if (auto Session = GetSession())
    {
        if (!OnFindCompleteHandle.IsValid())
        {
            OnFindCompleteHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(
                FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamSessionManager::HandleFindSessions));
        }
    }
}

void USteamSessionManager::RegisterJoinDelegate()
{
    if (auto Session = GetSession())
    {
        if (!OnJoinCompleteHandle.IsValid())
        {
            OnJoinCompleteHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
                FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamSessionManager::HandleJoinSession));
        }
    }
}

void USteamSessionManager::RegisterDestroyDelegate()
{
    if (auto Session = GetSession())
    {
        if (!OnDestroyCompleteHandle.IsValid())
        {
            OnDestroyCompleteHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
                FOnDestroySessionCompleteDelegate::CreateUObject(this, &USteamSessionManager::HandleDestroySession));
        }
    }
}

void USteamSessionManager::ClearAllDelegates()
{
    if (auto Session = GetSession())
    {
        if (OnCreateCompleteHandle.IsValid())
        {
            Session->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateCompleteHandle);
            OnCreateCompleteHandle.Reset();
        }
        if (OnFindCompleteHandle.IsValid())
        {
            Session->ClearOnFindSessionsCompleteDelegate_Handle(OnFindCompleteHandle);
            OnFindCompleteHandle.Reset();
        }
        if (OnJoinCompleteHandle.IsValid())
        {
            Session->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinCompleteHandle);
            OnJoinCompleteHandle.Reset();
        }
        if (OnDestroyCompleteHandle.IsValid())
        {
            Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroyCompleteHandle);
            OnDestroyCompleteHandle.Reset();
        }
    }
}

// ===== Callbacks =====
void USteamSessionManager::HandleCreateSession(FName SessionName, bool bWasSuccessful)
{
    if (auto Session = GetSession())
    {
        Session->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateCompleteHandle);
        OnCreateCompleteHandle.Reset();
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("CreateSession failed: %s"), *SessionName.ToString());
        OnCreateSessionFailed.Broadcast();
        OnCreateSessionFailedBP.Broadcast();
        return;
    }

    // 성공
    OnCreateSessionSucceeded.Broadcast();
    OnCreateSessionSucceededBP.Broadcast();

    // 맵 이름 읽고 리슨 오픈
    FString MapName;
    if (const auto* Named = GetSession()->GetNamedSession(SessionName))
    {
        Named->SessionSettings.Get(KEY_MAP_NAME, MapName);
    }

    if (MapName.IsEmpty())
    {
        // 현재 맵을 리슨으로
        UGameplayStatics::OpenLevel(GetWorld(), *UGameplayStatics::GetCurrentLevelName(GetWorld(), true), true, TEXT("listen"));
    }
    else
    {
        UGameplayStatics::OpenLevel(GetWorld(), FName(*MapName), true, TEXT("listen"));
    }
}

void USteamSessionManager::HandleFindSessions(bool bWasSuccessful)
{
    if (auto Session = GetSession())
    {
        Session->ClearOnFindSessionsCompleteDelegate_Handle(OnFindCompleteHandle);
        OnFindCompleteHandle.Reset();
    }

    if (!bWasSuccessful || !SearchHandle.IsValid())
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("FindSessions failed or no search handle"));
        CachedSummaries.Reset();
        OnFindSessionsCompleted.Broadcast();
        OnFindSessionsCompletedBP.Broadcast();
        return;
    }

    BuildSummariesFromSearch();
    OnFindSessionsCompleted.Broadcast();
    OnFindSessionsCompletedBP.Broadcast();
}

void USteamSessionManager::HandleJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (auto Session = GetSession())
    {
        Session->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinCompleteHandle);
        OnJoinCompleteHandle.Reset();
    }

    // 네이티브 신호 필요하면 여기서도 호출 가능:
    // OnJoinCompleted.Broadcast(Result);

    const EJoinResultBP BP = ToBPJoinResult(Result);
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogSteamSessionMgr, Warning, TEXT("JoinSession failed: %d"), static_cast<int32>(Result));
        OnJoinCompletedBP.Broadcast(BP);
        return;
    }

    FString ConnectString;
    if (GetSession()->GetResolvedConnectString(SessionName, ConnectString))
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
            OnJoinCompletedBP.Broadcast(BP);
            return;
        }
    }

    OnJoinCompletedBP.Broadcast(EJoinResultBP::UnknownError);
}

void USteamSessionManager::HandleDestroySession(FName SessionName, bool bWasSuccessful)
{
    if (auto Session = GetSession())
    {
        Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroyCompleteHandle);
        OnDestroyCompleteHandle.Reset();
    }

    OnDestroySessionCompleted.Broadcast();
    OnDestroySessionCompletedBP.Broadcast();

    if (bPendingCreateAfterDestroy && bWasSuccessful)
    {
        UE_LOG(LogSteamSessionMgr, Log, TEXT("Recreating session after destroy..."));
        bPendingCreateAfterDestroy = false;
        StartCreateSessionNow(Pending_MaxPlayers, Pending_ServerName, Pending_MapName);
        return;
    }

    bPendingCreateAfterDestroy = false;
}

// ===== 유틸 =====
void USteamSessionManager::BuildSummariesFromSearch()
{
    CachedSummaries.Reset();
    if (!SearchHandle.IsValid()) return;

    for (const auto& Res : SearchHandle->SearchResults)
    {
        FSessionSummary S;
        Res.Session.SessionSettings.Get(KEY_SERVER_NAME, S.ServerName);
        Res.Session.SessionSettings.Get(KEY_MAP_NAME,    S.MapName);

        S.MaxPlayers = Res.Session.SessionSettings.NumPublicConnections;
        S.CurrentPlayers = S.MaxPlayers - Res.Session.NumOpenPublicConnections;
        S.PingMs = Res.PingInMs;

        CachedSummaries.Add(S);
    }
}

EJoinResultBP USteamSessionManager::ToBPJoinResult(EOnJoinSessionCompleteResult::Type R)
{
    using T = EOnJoinSessionCompleteResult::Type;
    switch (R)
    {
    case T::Success:                 return EJoinResultBP::Success;
    case T::AlreadyInSession:        return EJoinResultBP::AlreadyInSession;
    case T::SessionIsFull:           return EJoinResultBP::SessionIsFull;
    case T::SessionDoesNotExist:     return EJoinResultBP::SessionDoesNotExist;
    case T::CouldNotRetrieveAddress: return EJoinResultBP::CouldNotRetrieveAddress;
    default:                         return EJoinResultBP::UnknownError;
    }
}
