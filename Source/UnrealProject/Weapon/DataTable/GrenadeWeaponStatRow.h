// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjectileWeaponStatRow.h"
#include "GrenadeWeaponStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGrenadeWeaponStatRow : public FProjectileWeaponStatRow
{
	GENERATED_BODY()
public:
	// 발사 각도를 위로 올리는 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grenade")
	float LaunchPitchOffset = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Charge")
	float MaxChargeTime = 2.0f; // 최대 파워에 도달하는 시간 (예: 2초)

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Charge")
	float MinLaunchSpeed = 500.0f; // 클릭하자마자 뗄 때의 최소 속도 (톡 던지기)

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Charge")
	float MaxLaunchSpeed = 2000.0f; // 끝까지 모았을 때의 최대 속도 (풀파워 투척)
};
