// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../EnumTypes/UITypes.h"
#include "../EnumTypes/DirectorTypes.h"
#include "UnrealProjectPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class UNREALPROJECT_API AUnrealProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AUnrealProjectPlayerController();

	void ChangeInputContext(bool bIsDowned);

	void UpdateAndShowPrompt(const FText& ActionText, const FText& KeyText);
	void HidePrompt();

	void ShowFeedback(const FText& Message, EFeedbackType Type);

	// 게임 오버 UI 보여주는 함수
	void ShowGameOverUI();

	void OnGameCleared();

	UFUNCTION()
	void HandleDirectorPhaseChanged(EDirectorPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ChangeBGM(EDirectorPhase NewPhase, float FadeTime = 2.f);
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DownedMappingContext;
	// Begin Actor interface

public:
	class UInputMappingContext* GetDefaultIMC() const { return DefaultMappingContext; }
	class UInputMappingContext* GetDownedIMC() const { return DownedMappingContext; }

protected:

	virtual void BeginPlay() override;

protected:

	// Main Widget
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPlayerMainWidget> MainHUDWidgetClass;

	UPROPERTY()
	class UPlayerMainWidget* MainHUDInstance;

	// Phase Debug 확인용 위젯
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|DirectorDebug")
	TSubclassOf<class UDirectorDebugWidget> DirectorDebugWidgetClass;

	UPROPERTY()
	UDirectorDebugWidget* DirectorDebugWidgetInstance;

	// 상호작용 매니저 위젯
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Interaction")
	TSubclassOf<class UInteractionMgr> InteractionMgrWidgetClass;

	UPROPERTY()
	UInteractionMgr* InteractionMgrWidgetInstance;

	// 게임 오버 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|GameOver")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	// 게임 클리어
	UPROPERTY(EditAnywhere, Category = "UI|GameClear")
	TSubclassOf<UUserWidget> GameClearWidgetClass;

	UPROPERTY()
	UUserWidget* GameOverWidget;

	UPROPERTY()
	UUserWidget* GameClearWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TMap<EDirectorPhase, class USoundBase*> BGMList;

public:
	// 사운드 클래스와 믹스를 넣어줄 포인터
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundMix* MainSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundClass* MasterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* BGMClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* SFXClass;

	void SetMasterVolume(float NewVolume);
	void SetBGMVolume(float NewVolume);
	void SetSFXVolume(float NewVolume);

private:
	// 크로스 페이드를 위한 2개의 오디오 컴포넌트
	UPROPERTY()
	class UAudioComponent* AudioCompA;

	UPROPERTY()
	class UAudioComponent* AudioCompB;

	// 현재 재생 중인 컴포넌트 추적용
	class UAudioComponent* CurrentAudioComp;

	EDirectorPhase CurrentAudioPhase;
};
