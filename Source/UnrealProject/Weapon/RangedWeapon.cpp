// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"

ARangedWeapon::ARangedWeapon()
{
}

void ARangedWeapon::Attack()
{
	if (!CanFire()) {
		return;
	}
	Super::Attack();

	ConsumeAmmo();

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ARangedWeapon::Reload()
{
	if (CurrentAmmoInClip >= MaxAmmoPerClip || bIsReloading) {
		return;
	}

	// 재장전 시작
	bIsReloading = true;

	UE_LOG(LogTemp, Log, TEXT("Reloading..."));

	FTimerHandle ReloadTimerHandle;
	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&ARangedWeapon::FinishReload,
		ReloadTime,
		false
	);
}

void ARangedWeapon::FinishReload()
{
	bIsReloading = false; // 상태 해제
	CurrentAmmoInClip = MaxAmmoPerClip; // 탄약 채움
	UE_LOG(LogTemp, Log, TEXT("Reload Complete!"));
}

bool ARangedWeapon::CanFire() const
{
	// 재장전중
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Log, TEXT("Reloading... Cant Fire"));
		return false;
	}

	// 총알 부족
	if (CurrentAmmoInClip <= 0) {
		/*틱틱하는 소리 재생 필요(탄창 비어 있음을 표현)*/ 
		UE_LOG(LogTemp, Log, TEXT("Cant Fire Ammo Zero"));
		return false;
	}

	// 시간 계산해서 연사 속도 체크
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < AttackRate) {
		UE_LOG(LogTemp, Log, TEXT("Cant Fire So Fast"));
		return false;
	}

	return true;
}

void ARangedWeapon::ConsumeAmmo()
{
	if (CurrentAmmoInClip > 0) CurrentAmmoInClip--;
}
