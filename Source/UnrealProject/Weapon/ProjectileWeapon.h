// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
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
	virtual void OnAttack() override;

protected:
	// 투사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	TSubclassOf<class AUnrealProjectProjectile> ProjectileClass;

	UPROPERTY()
	class UObjectPoolComponent* ProjectilePool;
};
