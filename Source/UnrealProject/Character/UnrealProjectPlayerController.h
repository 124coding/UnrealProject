// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

	// 게임 오버 UI 보여주는 함수
	void ShowGameOverUI();
	
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
	UPROPERTY(EditDefaultsOnly, Category = "GameOver")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY()
	UUserWidget* GameOverWidget;
};
