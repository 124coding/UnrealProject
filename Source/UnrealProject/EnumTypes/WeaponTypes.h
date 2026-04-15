// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_AssaultRifle UMETA(DisplayName = "AR"),
	EAT_Launcher	 UMETA(DisplayName = "LC"),
	EAT_Shotgun      UMETA(DisplayName = "SG"),
	EAT_Sniper       UMETA(DisplayName = "SR"),
	EAT_Null		 UMETA(DisplayName = "Null"),
	EAT_MAX
};

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	Equipped	UMETA(DisplayName = "Equipped"),
	Dropped		UMETA(DisplayName = "Dropped")
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single	UMETA(DisplayName = "Single"),
	Burst	UMETA(DisplayName = "Burst"),
	Auto	UMETA(DisplayName = "Auto")
};

UENUM(BlueprintType)
enum class EWeaponSlot : uint8 {
	Primary		UMETA(DisplayName = "Primary Weapon"),
	Secondary	UMETA(DisplayName = "Secondary Weapon"),
	Melee		UMETA(DisplayName = "Melee Weapon"),
	Throwable	UMETA(DisplayName = "Throwable"),
	MAX			UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMeleeType : uint8 {
	Thrust	UMETA(DisplayName = "Thrust"),
	Sweep	UMETA(DisplayName = "Sweep")
};

UENUM(BlueprintType)
enum class EDamageMethod : uint8 {
	SingleTarget	UMETA(DisplayName = "SingleTarget"),
	RadialDamage	UMETA(DisplayName = "RadialDamage")
};