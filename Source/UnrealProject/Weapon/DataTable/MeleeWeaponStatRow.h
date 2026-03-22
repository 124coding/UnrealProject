// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../../EnumTypes/WeaponTypes.h"
#include "BaseWeaponStatRow.h"
#include "MeleeWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMeleeWeaponStatRow : public FBaseWeaponStatRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Melee")
	float TraceRadius = 20.0f;
};
