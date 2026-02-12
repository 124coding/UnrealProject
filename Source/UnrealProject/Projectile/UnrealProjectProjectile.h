// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../PoolableInterface.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer")
	float LifeSpanTime;

	// 수명 관리용 타이머
	FTimerHandle LifeSpanTimer;

protected:
	virtual void DealDamage(AActor* HitActor);

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

	// 타겟을 향해 날아가는 로직
	virtual void LaunchTowards(FVector StartLoc, AActor* TargetActor);
};

