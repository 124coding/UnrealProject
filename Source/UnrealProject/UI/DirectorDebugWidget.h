// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DirectorDebugWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UDirectorDebugWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CurrentPhase;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_StressLevel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_AliveEnemies;
	
	
};
