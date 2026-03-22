// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RangedWeaponStatRow.h"
#include "HitScanWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHitScanWeaponStatRow : public FRangedWeaponStatRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan")
	class UParticleSystem* ImpactParticles; // 벽/적에 맞았을 때 뜨는 이펙트 (나이아가라로 변경 가능)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan")
	class USoundBase* ImpactSound; // 맞았을 때 나는 소리

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan")
	class UParticleSystem* BeamParticles; // 총알 궤적 (스모크 트레일)
};
