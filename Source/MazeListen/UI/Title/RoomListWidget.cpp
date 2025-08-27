// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomListWidget.h"
#include "Online/SteamSessionManager.h"
#include "Components/ListView.h"
#include "Components/Button.h"

void URoomListWidget::RefreshSessionList(const TArray<FSessionSummary>& Results)
{
	SessionList->ClearListItems();

	for (int32 i = 0; i < Results.Num(); ++i)
	{
		// 간단히 FText로 넣을 수도 있지만, 보통은 사용자 정의 ListEntry(UserWidget) 만들고
		// 데이터 오브젝트(UObject 파생)로 아이템을 넣는 방식을 권장.
		// 여기서는 간단화를 위해 문자열만 예시:
		const FSessionSummary& S = Results[i];
		const FString Line = FString::Printf(TEXT("[%d] %s  %d/%d  %s  (%dms)"),
			i, *S.ServerName, S.CurrentPlayers, S.MaxPlayers, *S.MapName, S.PingMs);

		// ListView는 UObject* 아이템을 요구 → 간단 데이터용 UObject 만들거나,
		// 혹은 BP에서 ListEntry를 구성해서 Index만 넘긴 뒤 클릭 시 JoinLobbyByIndex(Index) 호출하도록 하자.
		// (여기선 Index만 기억하도록 BP에서 처리하는 쪽을 추천)
	}

	// 실전 팁: ListView의 EntryWidgetClass를 사용자 정의 항목 위젯으로 설정하고
	// 거기에 “참가” 버튼을 두어, OnClicked → Subsystem->JoinLobbyByIndex(Index) 호출.
}
