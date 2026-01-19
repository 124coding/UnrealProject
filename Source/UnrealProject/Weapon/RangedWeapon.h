// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "RangedWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API ARangedWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	ARangedWeapon();

	virtual void Attack() override;

	// 재장전
	virtual void Reload();
	void FinishReload();

protected:
	// 발사 가능 여부
	bool CanFire() const;
	
	// 총알 줄이는 함수
	void ConsumeAmmo();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Ammo")
	int32 MaxAmmoPerClip = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RangedWeapon|Ammo")
	int32 CurrentAmmoInClip = MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Stats")
	float ReloadTime = 1.5f;

	// 탄퍼짐 필요 시 고려
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Stats")
	//float BulletSpread = 1.5f; // 0이면 레이저, 높을수록 퍼짐

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|FX")
	FName MuzzleSocketName = TEXT("MuzzleSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "RangedWeapon|FX")
	class UParticleSystem* MuzzleFlashFX; // 나이아가라라면 UNiagaraSystem


protected:
	// 마지막으로 총을 쏜 시각
	double LastFireTime = 0.0f;

	// 장전 중인지 여부
	bool bIsReloading = false;

	// 사거리
	float AttackRange = 0.0f;
};
