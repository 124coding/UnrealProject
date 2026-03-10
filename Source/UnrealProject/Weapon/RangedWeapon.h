// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Camera/CameraShakeBase.h"

#include "../EnumTypes/WeaponTypes.h"

#include "RangedWeapon.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmo, int32, CurrentAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadFinished, int32, AmmoConsumed);

UCLASS()
class UNREALPROJECT_API ARangedWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	ARangedWeapon();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	virtual void ExecuteFire() override;

	virtual void StopAttack() override;

	// 재장전
	virtual bool CanReload();
	virtual void Reload(int32 AvailableReserveAmmo);

	void FinishReload();

	void SetMaxAmmoInClip(int32 SetCount) {
		MaxAmmoPerClip = SetCount;
	}

	int32 GetMaxAmmoInClip() {
		return MaxAmmoPerClip;
	}

	void MinusMaxAmmoInClip() {
		MaxAmmoPerClip--;
	}

	void PlusMaxAmmoInClip() {
		MaxAmmoPerClip++;
	}

	void SetCurrentAmmoInClip(int32 SetCount);

	int32 GetCurrentAmmoInClip() {
		return CurrentAmmoInClip;
	}

protected:
	// 발사 가능 여부
	virtual bool CanAttack() override;
	
	// 총알 줄이는 함수
	void ConsumeAmmo();

	// 화면 중앙 트레이스 함수
	bool GetCrosshairTarget(FVector& OutHitLocation);

	// 컴포넌트가 계산한 총알을 탄창에 채워넣는 함수
	void AddAmmoToClip(int32 AmmoToAdd);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Ammo")
	int32 MaxAmmoPerClip = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RangedWeapon|Ammo")
	int32 CurrentAmmoInClip = MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Stats")
	float ReloadTime = 1.5f;

	// 재장전 타이머가 끝날 때 탄창에 실제로 더해줄 임시 총알 수
	int32 AmmoToReload = 0;

	// 탄퍼짐 필요 시 고려
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|Stats")
	//float BulletSpread = 1.5f; // 0이면 레이저, 높을수록 퍼짐

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RangedWeapon|FX")
	FName MuzzleSocketName = TEXT("MuzzleSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "RangedWeapon|FX")
	class UNiagaraSystem* MuzzleFlashFX = nullptr; // 나이아가라라면 UNiagaraSystem

	// 카메라 쉐이크
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class UCameraShakeBase> FireCameraShakeClass;

	// Recoil Curve
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	class UCurveFloat* RecoilCurve;

	// 반동이 얼마나 빠르게 적용될지
	UPROPERTY(EditAnywhere, Category = "Combat")
	float RecoilInterpSpeed = 15.0f;

protected:

	// 장전 중인지 여부
	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;

	// 몇 발 연속 쏘고 있는지
	int32 BurstCount = 0;

	// 최종적으로 도달해야 할 반동 목표치
	float TargetRecoilPitch = 0.0f;

	// 현재 화면에 적용된 반동 수치 (Target을 천천히 따라감)
	float CurrentRecoilPitch = 0.0f;

public:
	// 장전을 강제로 멈추는 함수
	void CancelReload();

	// 이 무기가 소모하는 총알의 종류
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Ammo")
	EAmmoType WeaponAmmoType = EAmmoType::EAT_AssaultRifle;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAmmo OnAmmoDelegate;

	// 장전 완료 델리게이트 변수
	UPROPERTY()
	FOnWeaponReloadFinished OnWeaponReloadFinished;
};
