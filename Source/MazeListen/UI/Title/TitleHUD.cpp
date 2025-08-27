// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleHUD.h"

#include "RoomListWidget.h"
#include "Online/SteamSessionManager.h"
#include "UI/Title/TitleWidget.h"
#include "Components/Button.h"
#include "UI/LoadingWidget.h"
#include "Kismet/GameplayStatics.h"

void UTitleHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Title 버튼 핸들러
	if (TitleWidget)
	{
		TitleWidget->CreateButton->OnClicked.AddDynamic(this, &UTitleHUD::OnClicked_Create);
		TitleWidget->FindButton->OnClicked.AddDynamic(this, &UTitleHUD::OnClicked_Find);
	}
	
	if (RoomListWidget)
	{
		RoomListWidget->RefreshButton->OnClicked.AddDynamic(this, &UTitleHUD::OnClicked_Find);
	}
}

void UTitleHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 세션 매니저 가져오기
	if (UGameInstance* GI = GetGameInstance())
	{
		SessionMgr = GI->GetSubsystem<USteamSessionManager>();
	}

	// SOS 델리게이트 바인딩 (BP용)
	if (SessionMgr.IsValid())
	{
		SessionMgr->OnCreateSessionSucceededBP.AddDynamic(this, &UTitleHUD::HandleCreateSuccessBP);
		SessionMgr->OnCreateSessionFailedBP.AddDynamic(this, &UTitleHUD::HandleCreateFailBP);
		SessionMgr->OnFindSessionsCompletedBP.AddDynamic(this, &UTitleHUD::HandleFindCompletedBP);
		SessionMgr->OnJoinCompletedBP.AddDynamic(this, &UTitleHUD::HandleJoinCompletedBP);
		SessionMgr->OnDestroySessionCompletedBP.AddDynamic(this, &UTitleHUD::HandleDestroyCompletedBP);
	}

	SetLoading(false);
}

void UTitleHUD::NativeDestruct()
{
	Super::NativeDestruct();

	// SOS 델리게이트 언바인딩
	if (SessionMgr.IsValid())
	{
		SessionMgr->OnCreateSessionSucceededBP.RemoveAll(this);
		SessionMgr->OnCreateSessionFailedBP.RemoveAll(this);
		SessionMgr->OnFindSessionsCompletedBP.RemoveAll(this);
		SessionMgr->OnJoinCompletedBP.RemoveAll(this);
		SessionMgr->OnDestroySessionCompletedBP.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UTitleHUD::HandleCreateSuccessBP()
{
	// 로딩 끄기(레벨 로드 들어가면 곧 화면 전환)
	SetLoading(false);
}

void UTitleHUD::HandleCreateFailBP()
{
	SetLoading(false);
	// 필요하면 토스트/메시지 출력(UMG에서 TextBlock 바인딩해서 노출)
}

void UTitleHUD::HandleFindCompletedBP()
{
	SetLoading(false);
	RefreshSessionList();
}

void UTitleHUD::HandleJoinCompletedBP(EJoinResultBP Result)
{
	SetLoading(false);
	// Success면 ClientTravel이 이미 실행됨
	// 실패 결과에 따른 메시지 노출 등
}

void UTitleHUD::HandleDestroyCompletedBP()
{
	// 필요하면 타이틀 복귀 후 UI 초기화 등
}

void UTitleHUD::OnClicked_Create()
{
	if (!SessionMgr.IsValid()) return;
	SetLoading(true);
	SessionMgr->CreateLobby(MaxPlayers, DefaultServerName, LobbyMapName);
	// 성공 시 레벨 전환은 SessionManager가 HandleCreateSession()에서 수행
}

void UTitleHUD::OnClicked_Find()
{
	if (!SessionMgr.IsValid()) return;
	SetLoading(true);

	// OnlineSubsystem에서 100개 찾기
	SessionMgr->FindLobbies(false, 100);
}

void UTitleHUD::SetLoading(bool bShow)
{
	if (LoadingWidget)
	{
		LoadingWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UTitleHUD::RefreshSessionList()
{
	// RoomListWidget의 SessionList 업데이트 해주기
	if (RoomListWidget)
	{
		RoomListWidget->SetVisibility(ESlateVisibility::Visible);
		TitleWidget->SetVisibility(ESlateVisibility::Collapsed);
		const TArray<FSessionSummary>& Results = SessionMgr->GetLastSearchSummaries();
		RoomListWidget->RefreshSessionList(Results);
	}
}
