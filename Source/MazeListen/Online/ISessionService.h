#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Util/EnumTypes.h"          // FSessionSummary, EJoinResultBP
#include "ISessionService.generated.h"

// === BP용 Multicast Delegates (한 곳에서만 선언해야함) ===
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionSimpleBP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinCompletedBP, EJoinResultBP, Result);

UINTERFACE(BlueprintType)
class MAZELISTEN_API USessionService : public UInterface
{
	GENERATED_BODY()
};

/**
 * Steam / Null / (추후 EOS 등) 공통으로 맞추는 인터페이스.
 * USteamSessionManager, UNullSteamSessionManager 모두 구현.
 */
class MAZELISTEN_API ISessionService
{
	GENERATED_BODY()

public:
	// === 外部 API (USteamSessionManager와 동일 시그니처) ===
	virtual void CreateLobby(int32 MaxPlayers, const FString& ServerName, const FString& MapName) = 0;
	virtual void FindLobbies(bool bLanQuery = false, int32 MaxResults = 100) = 0;
	virtual void JoinLobbyByIndex(int32 Index) = 0;
	virtual void DestroyLobby() = 0;

	virtual const TArray<FSessionSummary>& GetLastSearchSummaries() const = 0;

	// === BP 이벤트 접근자가 꼭 필요하진 않지만, 원하면 추가 가능 ===
	// (이번 구현은 클래스의 UPROPERTY(BlueprintAssignable)로 바로 바인딩하도록 동일하게 맞춤)
};