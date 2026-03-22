// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BaseWeaponStatRow.h"
#include "RangedWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FRangedWeaponStatRow : public FBaseWeaponStatRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Ammo")
	int32 MaxAmmoPerClip = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Stats")
	float ReloadTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Ranged|FX")
	class UNiagaraSystem* MuzzleFlashFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Ranged")
	TSubclassOf<class UCameraShakeBase> FireCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	class UCurveFloat* RecoilCurve;

	UPROPERTY(EditAnywhere, Category = "Ranged")
	float RecoilInterpSpeed = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Ammo")
	EAmmoType WeaponAmmoType = EAmmoType::EAT_AssaultRifle;
};
