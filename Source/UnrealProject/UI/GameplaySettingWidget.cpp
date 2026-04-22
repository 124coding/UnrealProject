// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySettingWidget.h"
#include "../SystemSaveGame.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "KeyBindEntryWidget.h"
#include "../Character/UnrealProjectPlayerController.h"
#include "../Character/UnrealProjectCharacter.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"

void UGameplaySettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Slider_MouseSens)
	{
		Slider_MouseSens->OnValueChanged.AddDynamic(this, &UGameplaySettingWidget::OnMouseSensChanged);
	}
}

void UGameplaySettingWidget::OnMouseSensChanged(float Value)
{
	if (SaveData)
	{
		// 실시간으로 메모리의 감도 값만 변경
		SaveData->MouseSensitivity = Value;
	}
}

void UGameplaySettingWidget::RebindActionKey(UInputAction* TargetAction, FName MappingName, FKey OldKey, FKey NewKey)
{
	if (SaveData) {
		SaveData->KeyBindings.Add(MappingName, NewKey);
	}
}

void UGameplaySettingWidget::ApplyKeyBindingsToIMC()
{
	if (!DefaultIMC || !SaveData) return;

	AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(GetOwningPlayer());
	if (!PC) return;

	TArray<UInputMappingContext*> ContextsToUpdate;
	if (PC->GetDefaultIMC()) ContextsToUpdate.Add(PC->GetDefaultIMC());
	if (PC->GetDownedIMC()) ContextsToUpdate.Add(PC->GetDownedIMC());

	for (auto& KVP : SaveData->KeyBindings) {
		FName TargetMappingName = KVP.Key;
		FKey NewKey = KVP.Value;

		for (UInputMappingContext* IMC : ContextsToUpdate) {
			for (const FEnhancedActionKeyMapping& Mapping : IMC->GetMappings()) {
				bool bIsMatch = false;
				if (UPlayerMappableKeySettings* MapSettings = Mapping.GetPlayerMappableKeySettings()) {
					bIsMatch = (MapSettings->Name == TargetMappingName);
				}
				else {
					bIsMatch = (Mapping.PlayerMappableOptions.Name == TargetMappingName);
				}

				if (bIsMatch) {
					// 변경된 키로 덮어쓰기
					const_cast<FEnhancedActionKeyMapping&>(Mapping).Key = NewKey;
					break;
				}
			}
		}

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(PC->GetPawn());

			if (Player && Player->IsDowned()) {
				Subsystem->RemoveMappingContext(PC->GetDownedIMC());
				Subsystem->AddMappingContext(PC->GetDownedIMC(), 0);
			}
			else {
				Subsystem->RemoveMappingContext(PC->GetDefaultIMC());
				Subsystem->AddMappingContext(PC->GetDefaultIMC(), 0);
			}
		}
	}
}

FKey UGameplaySettingWidget::FindKeyForAction(UInputMappingContext* IMC, UInputAction* TargetAction, FName MappingName)
{
	if (!IMC || !TargetAction) return EKeys::Invalid;

	const TArray<FEnhancedActionKeyMapping>& Mappings = IMC->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : Mappings) {
		if (Mapping.Action == TargetAction) {
			if (UPlayerMappableKeySettings* MapSettings = Mapping.GetPlayerMappableKeySettings()) {
				if (MapSettings->Name == MappingName) return Mapping.Key;
			}
			else if (Mapping.PlayerMappableOptions.Name == MappingName) {
				return Mapping.Key; // 찾으면 해당 키 반환
			}
		}
	}

	return EKeys::Invalid;
}

void UGameplaySettingWidget::InitGameplaySettings(USystemSaveGame* InSettings)
{
	SaveData = InSettings;

	// 초기 설정 값으로 슬라이더 위치 맞추기
	if (SaveData && Slider_MouseSens) {
		Slider_MouseSens->SetValue(SaveData->MouseSensitivity);
	}

	if (!ScrollBox_KeyBinds || !KeyBindEntryClass || !DefaultIMC) return;

	if (SaveData && DefaultIMC) {

		TMap<FName, FKey> LoadedKeyBindings = SaveData->KeyBindings;

		for (auto& KVP : LoadedKeyBindings) {
			FName SavedMappingName = KVP.Key;
			FKey SavedKey = KVP.Value;

			// RebindableActions 배열에서 이름이 일치하는 Action 찾기
			for (FKeyBindData& BindData : RebindableActions) {
				if (BindData.MappingName == SavedMappingName) {
					// 기존 키 지우고 덮어쓰기
					FKey OldKey = FindKeyForAction(DefaultIMC, BindData.Action, BindData.MappingName);
					RebindActionKey(BindData.Action, BindData.MappingName, OldKey, SavedKey);
					break;
				}
			}
		}
	}

	ScrollBox_KeyBinds->ClearChildren();

	// 배열에 액션들 순회
	for (FKeyBindData& BindData : RebindableActions) {
		if (BindData.Action) {
			UKeyBindEntryWidget* NewEntry = CreateWidget<UKeyBindEntryWidget>(this, KeyBindEntryClass);
		
			if (NewEntry) {
				// 현재 IMC에서 이 액션이 무슨 키인지 찾기
				FKey CurrentKey = FindKeyForAction(DefaultIMC, BindData.Action, BindData.MappingName);
				NewEntry->InitKeyBindEntry(BindData.Action, BindData.MappingName, BindData.DisplayName, CurrentKey);
			
				NewEntry->OnKeyRebound.AddDynamic(this, &UGameplaySettingWidget::RebindActionKey);

				// 스크롤 박스에 넣기
				ScrollBox_KeyBinds->AddChild(NewEntry);
			}
		}
	}
}
