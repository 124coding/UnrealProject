// Copyright Epic Games, Inc. All Rights Reserved.


#include "UnrealProjectPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"

void AUnrealProjectPlayerController::ChangeInputContext(bool bIsDowned)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem) return;

	// 기존 매핑 지우기
	// ClearAllMappings는 UI 입력 등 다른 필수 입력까지 지울 위험이 있으므로, 
	// 특정 컨텍스트만 제거하는 방식을 사용하거나 상황에 맞춰 초기화할 필요성이 있음(현재는 코드의 구현 및 테스트를 용이하게 하기 위함)
	Subsystem->ClearAllMappings();
	
	if (bIsDowned) {
		if (DownedMappingContext) Subsystem->AddMappingContext(DownedMappingContext, 0);
	}
	else {
		if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AUnrealProjectPlayerController::ShowGameOverUI()
{
	if (GameOverWidgetClass) {
		GameOverWidget = CreateWidget<UUserWidget>(this, GameOverWidgetClass);
		if (GameOverWidget) {
			GameOverWidget->AddToViewport(10);	// Z-Order를 높게 줘서 HUD보다 위에 뜨게

			// 마우스 커서 보이기
			bShowMouseCursor = true;

			// 입력 모드를 UI전용으로 변경
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameOverWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
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