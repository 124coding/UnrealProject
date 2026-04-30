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

void UGameplaySettingWidget::ApplyGameplaySetting()
{
	if (!DefaultIMC || !SaveData) return;

	AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(GetOwningPlayer());
	if (!PC) return;

	// 마우스 감도 등 즉시 반영 가능한 스탯 갱신
	if (AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(PC->GetPawn())) {
		Player->SetMouseSensitivity(SaveData->MouseSensitivity);
	}

	// 동기화가 필요한 모든 IMC 에셋 수집 (데이터 파편화 방지)
	TArray<UInputMappingContext*> ContextsToUpdate;
	if (PC->GetSystemIMC()) ContextsToUpdate.Add(PC->GetSystemIMC());
	if (PC->GetDefaultIMC()) ContextsToUpdate.Add(PC->GetDefaultIMC());
	if (PC->GetDownedIMC()) ContextsToUpdate.Add(PC->GetDownedIMC());

	// 세이브 데이터 기반의 전수 조사 및 일괄 갱신
	for (auto& KVP : SaveData->KeyBindings) {
		FName TargetMappingName = KVP.Key;
		FKey NewKey = KVP.Value;

		// 등록된 모든 IMC를 순회하며 동기화
		for (UInputMappingContext* IMC : ContextsToUpdate) {

			// Action Name이 일치하는 매핑 탐색
			for (const FEnhancedActionKeyMapping& Mapping : IMC->GetMappings()) {
				bool bIsMatch = false;

				// Mappable Key Settings의 존재 여부에 따른 유연한 이름 비교
				if (UPlayerMappableKeySettings* MapSettings = Mapping.GetPlayerMappableKeySettings()) {
					bIsMatch = (MapSettings->Name == TargetMappingName);
				}
				else {
					bIsMatch = (Mapping.PlayerMappableOptions.Name == TargetMappingName);
				}

				if (bIsMatch) {
					// const_cast를 활용하여 읽기 전용(ReadOnly) 에셋의 데이터 보호를 해제하고 런타임 갱신
					const_cast<FEnhancedActionKeyMapping&>(Mapping).Key = NewKey;
					break;
				}
			}
		}

		// 입력 서브시스템 플러시 (Flush)
		// 메모리 상의 데이터는 변경되었으나, 서브시스템의 캐시를 갱신하기 위해 컨텍스트를 재등록함
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(PC->GetPawn());

			// 시스템 공통 조작계 갱신
			if (PC->GetSystemIMC()) {
				Subsystem->RemoveMappingContext(PC->GetSystemIMC());
				Subsystem->AddMappingContext(PC->GetSystemIMC(), 1);
			}

			// 캐릭터 상태(Downed 유무)에 따른 상황별 조작계 갱신
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
	if (!InSettings) return;

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
