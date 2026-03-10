// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

/*
	Player
*/

UENUM(BlueprintType)
enum class EPlayerState : uint8 {
	EPS_Normal UMETA(DisplayName = "Normal"), // 정상
	EPS_Downed UMETA(DisplayName = "Downed"), // 기절
	EPS_Dead UMETA(DisplayName = "Dead"), // 사망
};

/*
	Enemy
*/
UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	EES_Normal		UMETA(DisplayName = "Normal"),			// 평상시
	EES_Attacking	UMETA(DisplayName = "Attacking"),		// 공격 중
	EES_Stunned		UMETA(DisplayName = "Stunned"),			// 기절 
	EES_Dead		UMETA(DisplayName = "Dead")				// 사망
};

UENUM(BlueprintType)
enum class EEnemyType : uint8 {
	Melee		    UMETA(DisplayName = "Melee"),
	Ranged			UMETA(DisplayName = "Ranged")
};