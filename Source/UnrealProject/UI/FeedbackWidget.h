// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../EnumTypes/UITypes.h"
#include "FeedbackWidget.generated.h"

/**
 * 
 */

UCLASS()
class UNREALPROJECT_API UFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION()
    void SetFeedbackText(const FText& Feedback);

    void PlayFeedbackAnimation(EFeedbackType Type);

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Feedback;

    // 애니메이션 바인딩
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* Anim_Positive;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* Anim_Negative;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* Anim_Neutral;
};
