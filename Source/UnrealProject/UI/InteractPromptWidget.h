// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPromptWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UInteractPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void UpdatePromptText(const FText& KeyText, const FText& ActionText);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_KeyPrompt;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_ActionPrompt;
};
