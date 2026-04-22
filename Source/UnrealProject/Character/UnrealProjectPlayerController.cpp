// Copyright Epic Games, Inc. All Rights Reserved.


#include "UnrealProjectPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "../Component/DroneComponent.h"
#include "../UnrealProjectGameMode.h"
#include "../UI/PlayerMainWidget.h"
#include "../UI/DirectorDebugWidget.h"
#include "../UI/InteractionMgr.h"
#include "Kismet/GameplayStatics.h"
#include "../SystemSaveGame.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"

AUnrealProjectPlayerController::AUnrealProjectPlayerController()
{
	AudioCompA = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioCompA"));
	AudioCompA->SetupAttachment(RootComponent);
	AudioCompA->bAutoActivate = false;

	AudioCompB = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioCompB"));
	AudioCompB->SetupAttachment(RootComponent);
	AudioCompB->bAutoActivate = false;

	// 초기 설정
	CurrentAudioComp = AudioCompA;

	CurrentAudioPhase = EDirectorPhase::None;
}

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

void AUnrealProjectPlayerController::UpdateAndShowPrompt(const FText& ActionText, const FText& KeyText)
{
	if (InteractionMgrWidgetInstance) {
		InteractionMgrWidgetInstance->UpdateAndShowPrompt(ActionText, KeyText);
	}
}

void AUnrealProjectPlayerController::HidePrompt()
{
	if (InteractionMgrWidgetInstance) {
		InteractionMgrWidgetInstance->HidePrompt();
	}
}

void AUnrealProjectPlayerController::ShowFeedback(const FText& Message, EFeedbackType Type)
{
	if (InteractionMgrWidgetInstance) {
		InteractionMgrWidgetInstance->ShowFeedback(Message, Type);
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

void AUnrealProjectPlayerController::OnGameCleared()
{

	if (GameClearWidgetClass) {
		GameClearWidget = CreateWidget<UUserWidget>(this, GameClearWidgetClass);
		if (GameClearWidget) {
			GameClearWidget->AddToViewport(10);	// Z-Order를 높게 줘서 HUD보다 위에 뜨게

			// 마우스 커서 보이기
			bShowMouseCursor = true;

			// 입력 모드를 UI전용으로 변경
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameClearWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void AUnrealProjectPlayerController::HandleDirectorPhaseChanged(EDirectorPhase NewPhase)
{
	if (NewPhase == EDirectorPhase::Peak) {
		ChangeBGM(NewPhase, 1.f); // Peak는 좀 긴박하게
	}
	else {
		ChangeBGM(NewPhase);
	}
}

void AUnrealProjectPlayerController::ChangeBGM(EDirectorPhase NewPhase, float FadeTime)
{
	if (CurrentAudioPhase == NewPhase) return;
	if (!BGMList.Contains(NewPhase)) return;

	USoundBase* NewBGM = BGMList[NewPhase];
	if (!NewBGM) return;
	
	USoundBase* CurrentBGM = BGMList[CurrentAudioPhase];
	if (CurrentBGM) {
		if (NewBGM == CurrentBGM) return;
	}

	// 교대할 오디오 컴포넌트 찾기
	UAudioComponent* NextAudioComp = (CurrentAudioComp == AudioCompA) ? AudioCompB : AudioCompA;

	// 기존 음악은 페이드 아웃
	if (CurrentAudioComp->IsPlaying()) {
		if (FadeTime > 0.f) {
			CurrentAudioComp->FadeOut(FadeTime, 0.f);
		}
		else {
			CurrentAudioComp->Stop();
		}
	}

	// 새 음악은 다른 컴포넌트에 세팅하고 페이드 인
	NextAudioComp->SetSound(NewBGM);
	if (FadeTime > 0.f) {
		NextAudioComp->FadeIn(FadeTime, 1.f);
	}
	else {
		NextAudioComp->SetVolumeMultiplier(1.f);
		NextAudioComp->Play();
	}

	// 상태 업데이트
	CurrentAudioComp = NextAudioComp;
	CurrentAudioPhase = NewPhase;
}

void AUnrealProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::DoesSaveGameExist(TEXT("SystemSettings"), 0)) {
		if (USystemSaveGame* SaveData = Cast<USystemSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SystemSettings"), 0))) {

			TArray<UInputMappingContext*> ContextsToUpdate;
			if (DefaultMappingContext) ContextsToUpdate.Add(DefaultMappingContext);
			if (DownedMappingContext) ContextsToUpdate.Add(DownedMappingContext);

			for (auto& KVP : SaveData->KeyBindings) {
				FName TargetMappingName = KVP.Key;
				FKey NewKey = KVP.Value;

				for (UInputMappingContext* IMC : ContextsToUpdate) {
					if (!IMC) continue;

					for (const FEnhancedActionKeyMapping& Mapping : IMC->GetMappings()) {
						bool bIsMatch = false;
						if (UPlayerMappableKeySettings* MapSettings = Mapping.GetPlayerMappableKeySettings()) {
							bIsMatch = (MapSettings->Name == TargetMappingName);
						}
						else {
							bIsMatch = (Mapping.PlayerMappableOptions.Name == TargetMappingName);
						}

						if (bIsMatch) {
							const_cast<FEnhancedActionKeyMapping&>(Mapping).Key = NewKey;
						}
					}
				}
			}
		}
	}

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (MainHUDWidgetClass)
	{
		MainHUDInstance = CreateWidget<UPlayerMainWidget>(this, MainHUDWidgetClass);
		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();

			if (APawn* MyPawn = GetPawn())
			{
				// 캐릭터가 소유한 드론 컴포넌트를 탐색
				UDroneComponent* DroneComp = MyPawn->FindComponentByClass<UDroneComponent>();

				if (DroneComp)
				{
					MainHUDInstance->InitializeHUD(DroneComp);
				}
			}
		}
	}

	if (DirectorDebugWidgetClass)
	{
		DirectorDebugWidgetInstance = CreateWidget<UDirectorDebugWidget>(this, DirectorDebugWidgetClass);
		if (DirectorDebugWidgetInstance)
		{
			DirectorDebugWidgetInstance->AddToViewport();
		}
	}

	if (InteractionMgrWidgetClass)
	{
		InteractionMgrWidgetInstance = CreateWidget<UInteractionMgr>(this, InteractionMgrWidgetClass);
		if (InteractionMgrWidgetInstance)
		{
			InteractionMgrWidgetInstance->AddToViewport();

			InteractionMgrWidgetInstance->HidePrompt();
			InteractionMgrWidgetInstance->HideFeedback();
		}
	}

	AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode());
	if (GM) {
		GM->OnPhaseChanged.AddDynamic(this, &AUnrealProjectPlayerController::HandleDirectorPhaseChanged);
		ChangeBGM(GM->CurrentPhase, 0.f);
	}

	bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(true);

	SetInputMode(InputMode);
}

void AUnrealProjectPlayerController::SetMasterVolume(float NewVolume)
{
	if (MainSoundMix && MasterClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MainSoundMix,
			MasterClass,
			NewVolume,
			1.0f,     
			0.1f,     
			true      
		);
	}
}

void AUnrealProjectPlayerController::SetBGMVolume(float NewVolume)
{
	if (MainSoundMix && BGMClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MainSoundMix,
			BGMClass,
			NewVolume,
			1.0f,     
			0.1f,     
			true      
		);
	}
}

void AUnrealProjectPlayerController::SetSFXVolume(float NewVolume)
{
	if (MainSoundMix && SFXClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MainSoundMix,
			SFXClass,
			NewVolume, 
			1.0f,      
			0.1f,      
			true       
		);
	}
}
