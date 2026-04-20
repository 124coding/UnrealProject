// Fill out your copyright notice in the Description page of Project Settings.


#include "FeedbackWidget.h"
#include "Components/TextBlock.h"

void UFeedbackWidget::SetFeedbackText(const FText& Feedback)
{
	if (Text_Feedback) {
		Text_Feedback->SetText(Feedback);
	}
}

void UFeedbackWidget::PlayFeedbackAnimation(EFeedbackType Type)
{
	if (Anim_Positive) StopAnimation(Anim_Positive);
	if (Anim_Negative) StopAnimation(Anim_Negative);
	if (Anim_Neutral)  StopAnimation(Anim_Neutral);

	switch (Type) {
	case EFeedbackType::Positive:
		if (Anim_Positive) {
			Text_Feedback->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
			PlayAnimation(Anim_Positive);
		}
		break;

	case EFeedbackType::Negative:
		if (Anim_Negative) {
			Text_Feedback->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)));
			PlayAnimation(Anim_Negative);
		}
		break;

	case EFeedbackType::Neutral:
		if (Anim_Neutral) {
			Text_Feedback->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));
			PlayAnimation(Anim_Neutral);
		}
		break;

	default:
		break;
	}
}
