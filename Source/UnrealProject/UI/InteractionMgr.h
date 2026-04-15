// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../EnumTypes/UITypes.h"
#include "InteractionMgr.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UInteractionMgr : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateAndShowPrompt(const FText& ActionText, const FText& KeyText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HidePrompt();


	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowFeedback(const FText& Message, EFeedbackType Type);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HideFeedback();

protected:
	UPROPERTY(meta = (BindWidget))
	class UInteractPromptWidget* WBP_InteractPrompt;

	UPROPERTY(meta = (BindWidget))
	class UFeedbackWidget* WBP_FeedbackWidget;

	// 피드백 UI 숨기기 타이머 핸들
	FTimerHandle FeedbackTimerHandle;
};
