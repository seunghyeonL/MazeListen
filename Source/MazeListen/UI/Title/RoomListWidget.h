// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoomListWidget.generated.h"

class UListView;
class UButton;

UCLASS()
class MAZELISTEN_API URoomListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UListView* SessionList;
	UPROPERTY(meta = (BindWidget)) UButton* RefreshButton;
	UPROPERTY(meta = (BindWidget)) UButton* TitleButton;

	void RefreshSessionList(const TArray<struct FSessionSummary>& Results); // Find 결과를 리스트에 채우기
};
