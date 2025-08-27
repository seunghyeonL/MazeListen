// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ISessionService.h"
#include "NullSteamSessionManager.generated.h"

UCLASS()
class MAZELISTEN_API UNullSteamSessionManager : public UGameInstanceSubsystem, public ISessionService
{
    GENERATED_BODY()
public:
    // ====== 외부 API (USteamSessionManager와 동일 시그니처/카테고리) ======
    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    virtual void CreateLobby(int32 MaxPlayers, const FString& ServerName, const FString& MapName) override;

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    virtual void FindLobbies(bool bLanQuery = false, int32 MaxResults = 100) override;

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    virtual void JoinLobbyByIndex(int32 Index) override;

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    virtual void DestroyLobby() override;

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    virtual const TArray<FSessionSummary>& GetLastSearchSummaries() const override { return CachedSummaries; }

    // ====== BP 바인딩용 이벤트 (이름/타입 동일) ======
    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnCreateSessionSucceededBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnCreateSessionFailedBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnFindSessionsCompletedBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnJoinCompletedBP OnJoinCompletedBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnDestroySessionCompletedBP;

    // ====== C++ 전용(가벼운) 이벤트도 동일하게 제공 ======
    FSimpleMulticastDelegate OnCreateSessionSucceeded;
    FSimpleMulticastDelegate OnCreateSessionFailed;
    FSimpleMulticastDelegate OnFindSessionsCompleted;
    FSimpleMulticastDelegate OnDestroySessionCompleted;

    // 키 이름(USteamSessionManager와 동일)
    static const FName SESSION_NAME;
    static const FName KEY_SERVER_NAME;
    static const FName KEY_MAP_NAME;

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    // Online Session Interface
    IOnlineSessionPtr GetSession() const;
    IOnlineSessionPtr SessionInterface;

    // Delegate handles
    FDelegateHandle OnCreateCompleteHandle;
    FDelegateHandle OnFindCompleteHandle;
    FDelegateHandle OnJoinCompleteHandle;
    FDelegateHandle OnDestroyCompleteHandle;

    // Search / Cache
    TSharedPtr<FOnlineSessionSearch> SearchHandle;
    TArray<FSessionSummary> CachedSummaries;

    // 파괴 후 재생성 예약 파라미터 (USteamSessionManager 패턴과 동일)
    bool bPendingCreateAfterDestroy = false;
    int32 Pending_MaxPlayers = 0;
    FString Pending_ServerName;
    FString Pending_MapName;

    // 내부 유틸
    void RegisterCreateDelegate();
    void RegisterFindDelegate();
    void RegisterJoinDelegate();
    void RegisterDestroyDelegate();
    void ClearAllDelegates();

    void StartCreateSessionNow(int32 MaxPlayers, const FString& ServerName, const FString& MapName);
    void BuildSummariesFromSearch();

    // 콜백
    void HandleCreateSession(FName SessionName, bool bWasSuccessful);
    void HandleFindSessions(bool bWasSuccessful);
    void HandleJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void HandleDestroySession(FName SessionName, bool bWasSuccessful);

    // BP enum 매핑 유틸
    static EJoinResultBP ToBPJoinResult(EOnJoinSessionCompleteResult::Type R);
};

