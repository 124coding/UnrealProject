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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HitScan|Effects")
	class UParticleSystem* DefaultImpactParticle; // 맵에 없는 재질을 맞췄을 때 뜰 기본 파티클

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan")
	TMap<TEnumAsByte<EPhysicalSurface>, class UParticleSystem*> ImpactParticleMap; // 벽/적에 맞았을 때 뜨는 이펙트 (나이아가라로 변경 가능)

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HitScan|Effects")
	class UMaterialInterface* DefaultDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|Effects")
	TMap<TEnumAsByte<EPhysicalSurface>, class UMaterialInterface*> DecalMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HitScan|Effects")
	class USoundBase* DefaultImpactSound; // 맵에 없는 재질을 맞췄을 때 날 기본 소리

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|Effects")
	TMap<TEnumAsByte<EPhysicalSurface>, class USoundBase*> ImpactSoundMap; // 맞았을 때 나는 소리

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan")
	class UParticleSystem* BeamParticles; // 총알 궤적 (스모크 트레일)
};
