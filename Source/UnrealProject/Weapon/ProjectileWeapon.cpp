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
	ProjectilePool->InitializePool(CurrentProjectileStat.ProjectileClass, CurrentProjectileStat.MaxAmmoPerClip, this);
}

void AProjectileWeapon::ExecuteFire()
{
	Super::ExecuteFire();

	if (!CurrentProjectileStat.ProjectileClass || !ProjectilePool) return;

	if (const UStaticMeshComponent* Mesh = WeaponMesh) {
		FVector SocketLocation = Mesh->GetSocketLocation(MuzzleSocketName);

		FVector LaunchDir; // 최종 발사 방향
		FVector HitTarget; // 조준점이 보고 있는 곳

		bool bHit = GetCrosshairTarget(HitTarget);

		if (bHit) {
			LaunchDir = CalculateLaunchDirection(SocketLocation, HitTarget);
		}
		else {
			// 완전 허공
			if (APlayerController* PC = Cast<APlayerController>(GetInstigatorController())) {
				LaunchDir = PC->PlayerCameraManager->GetCameraRotation().Vector();
			}
			else {
				LaunchDir = GetActorForwardVector(); // 방어 코드
			}
		}

		if (AActor* SpawnedActor = ProjectilePool->SpawnFromPool(SocketLocation, LaunchDir.Rotation())) {
			if (AUnrealProjectProjectile* Projectile = Cast<AUnrealProjectProjectile>(SpawnedActor)) {
				float CurrentSpeed = GetLaunchSpeed();
				Projectile->Launch(LaunchDir, CurrentSpeed);
			}
		}
	}
}

void AProjectileWeapon::InitWeaponData()
{
	if (!WeaponDataHandle.IsNull())
	{
		FProjectileWeaponStatRow* RowData = WeaponDataHandle.GetRow<FProjectileWeaponStatRow>(TEXT("ProjectileWeaponDataLookup"));

		if (RowData)
		{
			CurrentProjectileStat = *RowData;
			CurrentRangedStat = *RowData;
			CurrentWeaponStat = *RowData;
		}
	}
}

FVector AProjectileWeapon::CalculateLaunchDirection(FVector MuzzleLocation, FVector HitTarget)
{
	float DistToTarget = (HitTarget - MuzzleLocation).Size();

	if (DistToTarget < 100.0f)
	{
		return GetActorForwardVector(); // 너무 가까우면 정면
	}

	// 기본 직사 방향
	return (HitTarget - MuzzleLocation).GetSafeNormal();
}

float AProjectileWeapon::GetLaunchSpeed() const
{
	return -1.0f;
}
