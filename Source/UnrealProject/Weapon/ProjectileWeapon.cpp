// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "../Projectile/UnrealProjectProjectile.h"
#include "../Component/ObjectPoolComponent.h"

AProjectileWeapon::AProjectileWeapon()
{
	ProjectilePool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("ProjectilePool"));
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 풀 초기화
	ProjectilePool->InitializePool(ProjectileClass, MaxAmmoPerClip * 3, this);
}

void AProjectileWeapon::OnAttack()
{
	Super::OnAttack();

	if (!ProjectileClass || !ProjectilePool) return;

	if (const UStaticMeshComponent* Mesh = WeaponMesh) {
		FVector SocketLocation = Mesh->GetSocketLocation(MuzzleSocketName);

		FVector LaunchDir; // 최종 발사 방향
		FVector HitTarget; // 조준점이 보고 있는 곳

		GetCrosshairTarget(HitTarget);

		// 거리 계산
		float DistToTarget = (HitTarget - SocketLocation).Size();

		// 너무 가까우면 그냥 정면으로 발사
		if (DistToTarget < 100.0f)
		{
			LaunchDir = GetActorForwardVector();
		}
		else
		{
			LaunchDir = (HitTarget - SocketLocation).GetSafeNormal();
		}

		if (AActor* SpawnedActor = ProjectilePool->SpawnFromPool(SocketLocation, LaunchDir.Rotation())) {
			if (AUnrealProjectProjectile* Projectile = Cast<AUnrealProjectProjectile>(SpawnedActor)) {
				Projectile->Launch(LaunchDir);
			}
		}
	}
}
