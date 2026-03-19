// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SurvivalGameInstance.h"
#include "SurvivalSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API USurvivalSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	USurvivalSaveGame();

	// 하드에 영구 저장될 데이터들
	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FAttributeSaveData SavedAttributeData;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FCombatSaveData SavedCombatData;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Drone")
	FDroneStats SavedDroneStats;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Drone")
	EDroneActiveSkill SavedDroneActiveSkill;

	// 저장 슬롯 이름
	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FString SaveSlotName;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data|Progress")
	FName SavedLevelName;

	// 유저 인덱스 (보통 0 사용)
	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	int32 UserIndex;
};
