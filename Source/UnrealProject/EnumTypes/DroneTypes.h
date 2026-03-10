// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DroneTypes.generated.h"

UENUM(BlueprintType)
enum class EDroneState : uint8 {
	Idle,		// 평상 시
	Reviving	// 부활 작업 중
};

UENUM(BlueprintType)
enum class EDroneActiveSkill : uint8 {
	None		    UMETA(DisplayName = "None"),
	Knockback		UMETA(DisplayName = "Explosive Blast"),
	BerserkerBuff	UMETA(DisplayName = "BerserkerBuff") // 추후 구현
};

UENUM(BlueprintType)
enum class EDroneUpgradeType : uint8 {
	Range			UMETA(DisplayName = "Range"),
	Force			UMETA(DisplayName = "Knockback Force"),
	Cooldown		UMETA(DisplayName = "Cooldown Reduction"),
	UnlockAttack	UMETA(DisplayName = "Unlock Auto Attack"),
	UnlockHeal		UMETA(DisplayName = "Unlock Heal"),
	AttackDamage	UMETA(DisplayName = "Attack Damage"),
	AttackSpeed		UMETA(DisplayName = "Attack Speed"),
	HealAmount		UMETA(DisplayName = "Heal Amount"),
	HealInterval	UMETA(DisplayName = "HealInterval"),
	ReviveAmount	UMETA(DisplayName = "ReviveAmount")
};