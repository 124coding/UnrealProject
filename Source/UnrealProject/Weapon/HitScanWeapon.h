// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "DataTable/HitScanWeaponStatRow.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AHitScanWeapon : public ARangedWeapon
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void ExecuteFire() override;

protected:
	virtual void InitWeaponData() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitScan|Stat")
	FHitScanWeaponStatRow CurrentHitScanStat;

	// 무기에 영구적으로 부착해둘 빔 파티클 컴포넌트
	UPROPERTY()
	class UParticleSystemComponent* PooledBeamComponent;

	// 풀링용 임팩트 파티클 배열
	UPROPERTY()
	TArray<UParticleSystemComponent*> ImpactPool;

	int32 ImpactIndex = 0;

	UPROPERTY()
	TArray<class UDecalComponent*> DecalPool;

	// 데칼 관리 타이머
	TArray<FTimerHandle> DecalTimers;

	int32 DecalIndex = 0;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	//class UParticleSystem* ImpactParticles; // 벽/적에 맞았을 때 뜨는 이펙트 (나이아가라로 변경 가능)

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	//class USoundBase* ImpactSound; // 맞았을 때 나는 소리

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitScan|FX")
	//class UParticleSystem* BeamParticles; // 총알 궤적 (스모크 트레일)

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "HitScan|FX")
	FName BeamTargetParamName = TEXT("Target"); // 파티클 시스템 내부에서 목표 지점
};
