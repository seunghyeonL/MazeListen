// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/EnumTypes.h"
#include "TitleHUD.generated.h"

/**
 * 
 */
UCLASS()
class MAZELISTEN_API UTitleHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// 에디터/디자이너가 설정할 값들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Session")
	int32 MaxPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Session")
	FString DefaultServerName = TEXT("Room");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Session")
	FString LobbyMapName = TEXT("Lobby");

protected:
	// 세션 매니저 참조
	TWeakObjectPtr<class USteamSessionManager> SessionMgr;

	UPROPERTY(meta = (BindWidget)) class UTitleWidget* TitleWidget;
	UPROPERTY(meta = (BindWidget)) class URoomListWidget* RoomListWidget;
	UPROPERTY(meta = (BindWidget)) class ULoadingWidget* LoadingWidget;
	
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// SOS 이벤트 핸들러
	UFUNCTION() void HandleCreateSuccessBP();
	UFUNCTION() void HandleCreateFailBP();
	UFUNCTION() void HandleFindCompletedBP();
	UFUNCTION() void HandleJoinCompletedBP(EJoinResultBP Result);
	UFUNCTION() void HandleDestroyCompletedBP();

	// TitleWidget 핸들러
	UFUNCTION() void OnClicked_Create();
	UFUNCTION() void OnClicked_Find();

	// RoomList 핸들러
	

	UFUNCTION() void RefreshSessionList(); // Find 결과를 리스트에 채우기
	
	void SetLoading(bool bShow);
};
