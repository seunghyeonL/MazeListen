// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/EnumTypes.h"
#include "TitleWidget.generated.h"

class UButton;
class USteamSessionManager;

UCLASS()
class MAZELISTEN_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UButton* CreateButton;
	UPROPERTY(meta = (BindWidget)) UButton* FindButton;
};
