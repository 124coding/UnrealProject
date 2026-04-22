// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyBindEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/InputKeySelector.h"
#include "InputAction.h"

void UKeyBindEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 키 선택기 이벤트 묶기
	if (KeySelector)
	{
		KeySelector->OnKeySelected.AddDynamic(this, &UKeyBindEntryWidget::OnKeySelected);
	}
}

void UKeyBindEntryWidget::OnKeySelected(FInputChord SelectedKey)
{
	if (MyAction && OnKeyRebound.IsBound())
	{
		OnKeyRebound.Broadcast(MyAction, MyMappingName, MyOldKey, SelectedKey.Key);

		MyOldKey = SelectedKey.Key;
	}
}

void UKeyBindEntryWidget::InitKeyBindEntry(class UInputAction* InAction, FName InMappingName, FText InDisplayName, FKey InCurrentKey)
{
	MyAction = InAction;
	MyMappingName = InMappingName;
	MyOldKey = InCurrentKey;

	if (Text_ActionName) {
		Text_ActionName->SetText(InDisplayName);
	}

	if (KeySelector) {
		KeySelector->SetSelectedKey(FInputChord(InCurrentKey));
	}
}
