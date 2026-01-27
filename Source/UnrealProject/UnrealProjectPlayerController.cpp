// Copyright Epic Games, Inc. All Rights Reserved.


#include "UnrealProjectPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

void AUnrealProjectPlayerController::ChangeInputContext(bool bIsDowned)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem) return;

	// 기존 매핑 지우기
	Subsystem->ClearAllMappings();
	
	if (bIsDowned) {
		if (DownedMappingContext) Subsystem->AddMappingContext(DownedMappingContext, 0);
	}
	else {
		if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AUnrealProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}