// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../../EnumTypes/WeaponTypes.h"
#include "BaseWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBaseWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	EWeaponSlot WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float Damage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float AttackRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float AttackRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float KnockbackPower = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Base")
    EWeaponFireMode FireMode = EWeaponFireMode::Single;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    int32 MaxBurstCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float BurstFireRate = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base|Assets")
    USoundBase* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base|Assets")
    UAnimMontage* FireAnimation;
};
