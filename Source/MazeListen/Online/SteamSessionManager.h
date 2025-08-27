// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISessionService.h"
#include "Delegates/DelegateCombinations.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SteamSessionManager.generated.h"

UCLASS()
class MAZELISTEN_API USteamSessionManager : public UGameInstanceSubsystem, public ISessionService
{
	GENERATED_BODY()
public:
    // ====== 외부 API ======
    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    void CreateLobby(int32 MaxPlayers, const FString& ServerName, const FString& MapName);

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    void FindLobbies(bool bLanQuery = false, int32 MaxResults = 100);

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    void JoinLobbyByIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    void DestroyLobby();

    UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
    const TArray<FSessionSummary>& GetLastSearchSummaries() const { return CachedSummaries; }

    // ====== BP 바인딩용 이벤트 ======
    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnCreateSessionSucceededBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnCreateSessionFailedBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnFindSessionsCompletedBP; // 결과는 GetLastSearchSummaries()로

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnJoinCompletedBP OnJoinCompletedBP;

    UPROPERTY(BlueprintAssignable, Category="Steam|Sessions")
    FOnSessionSimpleBP OnDestroySessionCompletedBP;

    // ====== C++ 전용(가벼운) 이벤트 ======
    FSimpleMulticastDelegate OnCreateSessionSucceeded;
    FSimpleMulticastDelegate OnCreateSessionFailed;
    FSimpleMulticastDelegate OnFindSessionsCompleted;
    FSimpleMulticastDelegate OnDestroySessionCompleted;
    // 필요하면 파라미터 있는 네이티브 델리게이트도 추가 가능:
    // using FOnJoinCompletedNative = TMulticastDelegate<void(EOnJoinSessionCompleteResult::Type)>;
    // FOnJoinCompletedNative OnJoinCompleted;

    // 키 이름(필요 시 교체)
    static const FName SESSION_NAME;
    static const FName KEY_SERVER_NAME;
    static const FName KEY_MAP_NAME;

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    IOnlineSessionPtr GetSession() const;

    // Delegate handles
    FDelegateHandle OnCreateCompleteHandle;
    FDelegateHandle OnFindCompleteHandle;
    FDelegateHandle OnJoinCompleteHandle;
    FDelegateHandle OnDestroyCompleteHandle;

    // Callbacks
    void HandleCreateSession(FName SessionName, bool bWasSuccessful);
    void HandleFindSessions(bool bWasSuccessful);
    void HandleJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void HandleDestroySession(FName SessionName, bool bWasSuccessful);

    // 검색 핸들/캐시
    TSharedPtr<FOnlineSessionSearch> SearchHandle;
    TArray<FSessionSummary> CachedSummaries;

    // 파괴 후 재생성 예약 파라미터
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

    // BP enum 매핑 유틸
    static EJoinResultBP ToBPJoinResult(EOnJoinSessionCompleteResult::Type R);
};
