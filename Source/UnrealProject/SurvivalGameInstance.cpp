// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalGameInstance.h"
#include "SurvivalSaveGame.h"
#include "Kismet/GameplayStatics.h"

void USurvivalGameInstance::ResetCampaignData()
{
	// 플레이어 스탯 (Attribute) 초기화
	PlayerAttributeData.MaxHealth = 100.f;
	PlayerAttributeData.CurrentHealth = 100.f;


	// 전투 및 인벤토리 (Combat) 초기화
	PlayerCombatData.SavedCarriedWeapons.Empty();
	PlayerCombatData.CurrentEquippedSlot = EWeaponSlot::MAX;
	PlayerCombatData.CarriedAmmo.Empty();

	// 드론 (Drone) 데이터 초기화
	SavedDroneStats = FDroneStats();
	SavedDroneActiveSkill = EDroneActiveSkill::Knockback;
}

void USurvivalGameInstance::SaveGameToDisk(int SlotIndex)
{
	FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotIndex);

	USurvivalSaveGame* SaveObj = Cast<USurvivalSaveGame>(UGameplayStatics::CreateSaveGameObject(USurvivalSaveGame::StaticClass()));

	if (SaveObj) {
		// GI에 있는 데이터들을 SaveGame 객체로 복제
		SaveObj->SavedAttributeData = this->PlayerAttributeData;
		SaveObj->SavedCombatData = this->PlayerCombatData;
		SaveObj->SavedDroneStats = this->SavedDroneStats;
		SaveObj->SavedDroneActiveSkill = this->SavedDroneActiveSkill;
		SaveObj->SavedLevelName = this->SavedLevelName;

		// 하드에 .sav 파일로 굽기
		UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, SaveObj->UserIndex);
	}
}

void USurvivalGameInstance::LoadGameFromDisk(int SlotIndex)
{
	FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotIndex);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
		USurvivalSaveGame* LoadObj = Cast<USurvivalSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (LoadObj) {
			this->PlayerAttributeData = LoadObj->SavedAttributeData;
			this->PlayerCombatData = LoadObj->SavedCombatData;
			this->SavedDroneStats = LoadObj->SavedDroneStats;
			this->SavedDroneActiveSkill = LoadObj->SavedDroneActiveSkill;
			this->SavedLevelName = LoadObj->SavedLevelName;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Slot Empty"), *SlotName);
	}
}

void USurvivalGameInstance::ClearSaveData()
{
	FString SlotName = TEXT("Slot_1");

	// 슬롯 이름에 해당하는 세이브 파일을 하드디스크에서 삭제!
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	}
}