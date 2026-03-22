// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RangedWeaponStatRow.h"
#include "ProjectileWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FProjectileWeaponStatRow : public FRangedWeaponStatRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class AUnrealProjectProjectile> ProjectileClass;
};
