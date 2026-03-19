// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EnumTypes/WeaponTypes.h"
#include "Component/DroneComponent.h"
#include "SurvivalGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FAttributeSaveData {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadWrite)
	float CurrentHealth = 100.f;
};

USTRUCT(BlueprintType)
struct FSavedWeaponInfo {
	GENERATED_BODY()

	// 무기를 새로 스폰하기 위한 클래스 정보
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class ABaseWeapon> WeaponClass;

	// 탄창에 남아있는 총알 수 등 추가 가능
	UPROPERTY(BlueprintReadWrite)
	int32 MagazineAmmo = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxAmmo = 0;
};

USTRUCT(BlueprintType)
struct FCombatSaveData {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<EWeaponSlot, FSavedWeaponInfo> SavedCarriedWeapons;

	UPROPERTY(BlueprintReadWrite)
	EWeaponSlot CurrentEquippedSlot;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class ABaseWeapon> DefaultWeaponClass;

	UPROPERTY(BlueprintReadWrite)
	TMap<EAmmoType, int32> CarriedAmmo;
};

UCLASS()
class UNREALPROJECT_API USurvivalGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FAttributeSaveData PlayerAttributeData;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FCombatSaveData PlayerCombatData;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Drone")
	FDroneStats SavedDroneStats;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Drone")
	EDroneActiveSkill SavedDroneActiveSkill;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Progress")
	FName SavedLevelName;

	// 새로하기(New Game)를 눌렀을 때 데이터를 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ResetCampaignData();

public:
	// 하드디스크에 게임 저장
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void SaveGameToDisk(int SlotIndex);

	// 하드디스크에서 게임 불러오기
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void LoadGameFromDisk(int SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void ClearSaveData();
};
