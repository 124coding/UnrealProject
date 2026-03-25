// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/PoolableInterface.h"
#include "../EnumTypes/WeaponTypes.h"
#include "UnrealProjectProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AUnrealProjectProjectile : public AActor, public IPoolableInterface
{
	GENERATED_BODY()

public:
	AUnrealProjectProjectile();

protected:
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;
	
	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

protected:
	// 투사체의 기본 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float BaseDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer")
	float LifeSpanTime;

	// 수명 관리용 타이머
	FTimerHandle LifeSpanTimer;

	// 데미지 적용 방식
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	EDamageMethod DamageMethod = EDamageMethod::SingleTarget;

	// 폭발 반경
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float ExplosionRadius = 200.0f;

	// 광역 최소 데미지
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float MinimumDamage = 10.0f;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Damage", meta = (EditCondition = "DamageMethod == EDamageMethod::RadialDamage"))
	float InnerRadius = 100.0f; // 100% 데미지 반경

	// 폭발 조건 설정
	UPROPERTY(VisibleAnywhere, Category = "Combat|Explosion")
	bool bExplodeOnTimer = false; // true면 수류탄처럼 시간에 의해 폭발

	// 폭발 대기 시간 (수류탄 타이머)
	UPROPERTY(VisibleAnywhere, Category = "Combat|Explosion", meta = (EditCondition = "bExplodeOnTimer"))
	float ExplosionDelay = 3.0f;

	// 폭발 타이머 핸들
	FTimerHandle ExplosionTimerHandle;

protected:
	virtual void DealDamage(const FHitResult& HitResult);

	// 타이머가 다 되면 강제 폭발 함수
	void TimeOutExplode();

public:

	/** called when projectile hits something */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

public:
	// 풀링 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UObjectPoolComponent* OwningPoolComponent;

	virtual void OnPoolSpawned_Implementation() override;
	virtual void OnPoolReturned_Implementation() override;
	virtual void SetOwningPool_Implementation(UObjectPoolComponent* NewPool) override;

	// 풀로 돌아가는 함수
	void Deactivate();

	// 방향 벡터를 받아서 날아가는 로직
	virtual void Launch(FVector ShootDirection, float SpeedOverride = -1.0f);

	// 타겟을 향해 날아가는 로직
	virtual void LaunchTowards(FVector StartLoc, AActor* TargetActor);

	// 초기화 함수
	void InitProjectile(float tDamage, EDamageMethod tDamageMethod = EDamageMethod::SingleTarget, float tExplosionRadius = 0.f, float tMinimumDamage = 0.f, float tInnerRadius = 0.f, bool tbExplodeOnTimer = false, float tExplosionDelay = 0.f);
};

