// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/UnrealProjectCharacter.h"
#include "../Component/CombatComponent.h"

void AGrenadeWeapon::BeginPlay()
{
	Super::BeginPlay();

	MaxAmmoPerClip = 1;
	CurrentAmmoInClip = MaxAmmoPerClip;
}

FVector AGrenadeWeapon::CalculateLaunchDirection(FVector MuzzleLocation, FVector HitTarget) {
	// 부모 클래스(소총 방식)가 계산한 기본 방향을 가져옵니다.
	FVector BaseDirection = Super::CalculateLaunchDirection(MuzzleLocation, HitTarget);

	// 수류탄 전용 각도(Pitch)를 더해줍니다.
	if (LaunchPitchOffset > 0.0f)
	{
		FRotator LaunchRot = BaseDirection.Rotation();
		LaunchRot.Pitch += LaunchPitchOffset;

		return LaunchRot.Vector();
	}

	return BaseDirection;
}

float AGrenadeWeapon::GetLaunchSpeed() const
{
	// 차징 계산 속도를 넘겨줌
	UE_LOG(LogTemp, Warning, TEXT("Speed: %f"), CalculatedChargeSpeed);
	return CalculatedChargeSpeed;
}

void AGrenadeWeapon::OnAttack()
{
	// 현재 시간을 기록
	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	/* 캐릭터가 팔을 뒤로 당기는 애니메이션 실행 */
}

void AGrenadeWeapon::StopAttack()
{
	if (!bIsCharging) return;
	bIsCharging = false;

	// 누르고 있던 시간 계산
	float HeldTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;

	// 차지 비율 계산 (0.0 ~ 1.0 사이로 고정)
	float ChargeRatio = FMath::Clamp(HeldTime / MaxChargeTime, 0.0f, 1.0f);

	// 비율에 따라 최종 발사 속도 보간
	CalculatedChargeSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, ChargeRatio);

	/* 던지는 애니메이션 실행 */

	Super::ExecuteFire();

	MaxAmmoPerClip--;
	CurrentAmmoInClip = MaxAmmoPerClip;

	if (MaxAmmoPerClip <= 0) {
		if (AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(GetOwner())) {
			if (UCombatComponent* PlayerCombat = Player->FindComponentByClass<UCombatComponent>()) {
				PlayerCombat->EquipWeaponBySlot(EWeaponSlot::Primary);
			}
		}
	}
}