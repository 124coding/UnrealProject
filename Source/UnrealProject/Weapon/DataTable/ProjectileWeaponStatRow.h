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

	// 데미지 적용 방식
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	EDamageMethod DamageMethod = EDamageMethod::SingleTarget;

	// 폭발 반경
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float ExplosionRadius = 200.0f;

	// 광역 최소 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float MinimumDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float InnerRadius = 100.0f;	 // 100% 데미지 반경

	// 폭발 조건 설정
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Explosion")
	bool bExplodeOnTimer = false; // true면 수류탄처럼 시간에 의해 폭발

	// 폭발 대기 시간 (수류탄 타이머)
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Explosion", meta = (EditCondition = "bExplodeOnTimer"))
	float ExplosionDelay = 3.0f;
};
