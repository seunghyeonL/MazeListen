// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeListenGameMode.h"
#include "Character/MazeListenCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMazeListenGameMode::AMazeListenGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
