// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AHitScanWeapon : public ARangedWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Attack() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	class UParticleSystem* ImpactParticles; // 벽/적에 맞았을 때 떠지는 이펙트 (나이아가라로 변경 가능)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	class USoundBase* ImpactSound; // 맞았을 때 나는 소리

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	class UParticleSystem* BeamParticles; // 총알 궤적 (스모크 트레일)

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "HitScan|FX")
	FName BeamTargetParamName = TEXT("Target"); // 파티클 시스템 내부에서 목표 지점
};
