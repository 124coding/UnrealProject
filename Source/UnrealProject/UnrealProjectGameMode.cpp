// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectGameMode.h"
#include "UnrealProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUnrealProjectGameMode::AUnrealProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
