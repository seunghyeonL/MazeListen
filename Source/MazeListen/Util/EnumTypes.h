#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

// === BP용 결과 enum (Join) ===
UENUM(BlueprintType)
enum class EJoinResultBP : uint8
{
	Success            UMETA(DisplayName="Success"),
	AlreadyInSession   UMETA(DisplayName="AlreadyInSession"),
	SessionIsFull      UMETA(DisplayName="SessionIsFull"),
	SessionDoesNotExist UMETA(DisplayName="SessionDoesNotExist"),
	CouldNotRetrieveAddress UMETA(DisplayName="CouldNotRetrieveAddress"),
	UnknownError       UMETA(DisplayName="UnknownError")
};

// === 로비 리스트 요약 ===
USTRUCT(BlueprintType)
struct FSessionSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString ServerName;
	UPROPERTY(BlueprintReadOnly) FString MapName;
	UPROPERTY(BlueprintReadOnly) int32 CurrentPlayers = 0;
	UPROPERTY(BlueprintReadOnly) int32 MaxPlayers = 0;
	UPROPERTY(BlueprintReadOnly) int32 PingMs = 0;
};
