// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "DataTable/ProjectileWeaponStatRow.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AProjectileWeapon : public ARangedWeapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon();

	virtual void BeginPlay() override;
	virtual void ExecuteFire() override;

protected:
	virtual void InitWeaponData() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Stat")
	FProjectileWeaponStatRow CurrentProjectileStat;

	// 투사체 클래스
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	TSubclassOf<class AUnrealProjectProjectile> ProjectileClass;*/

	UPROPERTY()
	class UObjectPoolComponent* ProjectilePool;

	// 방향 계산 함수
	virtual FVector CalculateLaunchDirection(FVector MuzzleLocation, FVector HitTarget);

	// 속도 반환 함수
	virtual float GetLaunchSpeed() const;
};
