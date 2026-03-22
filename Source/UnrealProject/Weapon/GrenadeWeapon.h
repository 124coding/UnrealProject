// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "DataTable/GrenadeWeaponStatRow.h"
#include "GrenadeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AGrenadeWeapon : public AProjectileWeapon
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	virtual void InitWeaponData() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade|Stat")
	FGrenadeWeaponStatRow CurrentGrenadeStat;

	// 발사 각도를 위로 올리는 오프셋
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Grenade")
	//float LaunchPitchOffset = 15.0f;

	//UPROPERTY(EditDefaultsOnly, Category = "Combat|Charge")
	//float MaxChargeTime = 2.0f; // 최대 파워에 도달하는 시간 (예: 2초)

	//UPROPERTY(EditDefaultsOnly, Category = "Combat|Charge")
	//float MinLaunchSpeed = 500.0f; // 클릭하자마자 뗄 때의 최소 속도 (톡 던지기)

	//UPROPERTY(EditDefaultsOnly, Category = "Combat|Charge")
	//float MaxLaunchSpeed = 2000.0f; // 끝까지 모았을 때의 최대 속도 (풀파워 투척)

	// 내부 계산용 변수
	float ChargeStartTime = 0.0f;
	bool bIsCharging = false;

	// 수류탄 최종 속도 저장
	float CalculatedChargeSpeed = -1.0f;
	
	virtual FVector CalculateLaunchDirection(FVector MuzzleLocation, FVector HitTarget) override;

	virtual float GetLaunchSpeed() const override;

public:
	virtual bool CanReload() override { return false; }

	virtual void OnAttack() override;
	virtual void StopAttack() override;
};
