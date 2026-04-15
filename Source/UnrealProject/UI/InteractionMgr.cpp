// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionMgr.h"
#include "InteractPromptWidget.h"
#include "FeedbackWidget.h"

void UInteractionMgr::UpdateAndShowPrompt(const FText& ActionText, const FText& KeyText)
{
    if (WBP_InteractPrompt)
    {
        WBP_InteractPrompt->UpdatePromptText(ActionText, KeyText);
        WBP_InteractPrompt->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UInteractionMgr::HidePrompt()
{
    if (WBP_InteractPrompt)
    {
        WBP_InteractPrompt->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInteractionMgr::ShowFeedback(const FText& Message, EFeedbackType Type)
{
    if (WBP_FeedbackWidget)
    {
        // 텍스트 설정
        WBP_FeedbackWidget->SetFeedbackText(Message);
        WBP_FeedbackWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        // 타입에 따라 색상과 애니메이션을 다르게 재생!
        WBP_FeedbackWidget->PlayFeedbackAnimation(Type);

        GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);

        float feedbackTime = 2.f;

        // 현재는 모든 피드백 애니메이션 시간을 2초로 맞춰뒀기에 전부 다 2초, 타입에 따라 애니메이션 길이가 다르다면 수정 가능
        switch (Type) {
        case EFeedbackType::Positive:
            feedbackTime = 2.f;
            break;
        case EFeedbackType::Negative:
            feedbackTime = 2.f;
            break;
        case EFeedbackType::Neutral:
            feedbackTime = 2.f;
            break;
        default:
            break;
        }

        // 애니메이션 재생 시간에 맞춰 HideFeedback 예약
        GetWorld()->GetTimerManager().SetTimer(
            FeedbackTimerHandle,
            this,
            &UInteractionMgr::HideFeedback,
            feedbackTime,
            false
        );
    }
}

void UInteractionMgr::HideFeedback()
{
    if (WBP_FeedbackWidget)
    {
        WBP_FeedbackWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}
