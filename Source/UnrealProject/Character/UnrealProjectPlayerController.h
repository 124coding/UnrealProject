// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../EnumTypes/UITypes.h"
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
	void ChangeInputContext(bool bIsDowned);

	void UpdateAndShowPrompt(const FText& ActionText, const FText& KeyText);
	void HidePrompt();

	void ShowFeedback(const FText& Message, EFeedbackType Type);

	// 게임 오버 UI 보여주는 함수
	void ShowGameOverUI();

	void OnGameCleared();
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DownedMappingContext;
	// Begin Actor interface
protected:

	virtual void BeginPlay() override;


protected:
	// Phase Debug 확인용 위젯
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DirectorDebug")
	TSubclassOf<class UDirectorDebugWidget> DirectorDebugWidgetClass;

	UPROPERTY()
	UDirectorDebugWidget* DirectorDebugWidgetInstance;

	// 상호작용 매니저 위젯
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
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
};
