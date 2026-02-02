// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

ARangedWeapon::ARangedWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARangedWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 반동 처리가 필요할 때만 연산 (Target과 Current의 차이가 클 때)
	if (!FMath::IsNearlyEqual(CurrentRecoilPitch, TargetRecoilPitch, 0.001f)) {
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn) {
			APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
			if (PC) {
				// 보간(Interp)을 통해 이번 프레임에 도달해야 할 위치 계산
				float NewRecoilPitch = FMath::FInterpTo(
					CurrentRecoilPitch,
					TargetRecoilPitch,
					DeltaTime,
					RecoilInterpSpeed
				);

				// 이번 프레임에 움직여야 할 양
				float DeltaPitch = NewRecoilPitch - CurrentRecoilPitch;
				
				PC->AddPitchInput(DeltaPitch);

				CurrentRecoilPitch = NewRecoilPitch;
			}
		}
	}
}

void ARangedWeapon::Attack()
{
	if (!CanFire()) {
		return;
	}

	Super::Attack();

	ConsumeAmmo();

	LastFireTime = GetWorld()->GetTimeSeconds();

	if (MuzzleFlashFX) {
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashFX,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn) {
		APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
		if (PC && FireCameraShakeClass) {
			PC->ClientStartCameraShake(FireCameraShakeClass);

			float RecoilAmount = -0.5f; // RecoilCurve 부재 시 기본값
			float RecoilMultiplier = 1.0f; // 속도에 따른 에임 반동 패널티

			if (RecoilCurve) {
				RecoilAmount = RecoilCurve->GetFloatValue(BurstCount);
			}

			ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
			if (OwnerCharacter) {
				float CurrentSpeed = OwnerCharacter->GetVelocity().Size();

				if (OwnerCharacter->GetCharacterMovement()->IsFalling()) {
					RecoilMultiplier = 3.0f;
				}
				else if (CurrentSpeed > 600.0f) {
					RecoilMultiplier = 2.0f;
				}
				else if (CurrentSpeed > 400.0f) {
					RecoilMultiplier = 1.5f;
				}
				else if (OwnerCharacter->bIsCrouched) {
					RecoilMultiplier = 0.5f;
				}
			}

			TargetRecoilPitch += (RecoilAmount * RecoilMultiplier);

			// 랜덤 Yaw를 통한 좌우 반동
			float RandomYaw = FMath::RandRange(-0.1f, 0.1f) * RecoilMultiplier;
			PC->AddYawInput(RandomYaw);

			BurstCount++;
		}
	}
}

void ARangedWeapon::StopAttack()
{
	BurstCount = 0;
	TargetRecoilPitch = 0.0f;
	CurrentRecoilPitch = 0.0f;
}

void ARangedWeapon::Reload()
{
	if (CurrentAmmoInClip >= MaxAmmoPerClip || bIsReloading) {
		return;
	}

	// 재장전 시작
	bIsReloading = true;

	UE_LOG(LogTemp, Log, TEXT("Reloading..."));

	if (ReloadSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

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
