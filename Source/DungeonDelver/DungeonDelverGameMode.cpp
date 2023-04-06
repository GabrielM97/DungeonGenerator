// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeonDelverGameMode.h"
#include "DungeonDelverPlayerController.h"
#include "DungeonDelverCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADungeonDelverGameMode::ADungeonDelverGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ADungeonDelverPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}