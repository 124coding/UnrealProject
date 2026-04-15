// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractPromptWidget.h"
#include "Components/TextBlock.h"

void UInteractPromptWidget::UpdatePromptText(const FText& ActionText, const FText& KeyText)
{
	Text_KeyPrompt->SetText(KeyText);
	Text_ActionPrompt->SetText(ActionText);
}
