// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../UnrealProject.h"

ARangedWeapon::ARangedWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARangedWeapon::BeginPlay()
{
	Super::BeginPlay();
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

void ARangedWeapon::OnAttack()
{
	Super::OnAttack();

	ConsumeAmmo();

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

			// 랜덤 Yaw를 통한 좌우 반동 (이것도 따로 그래프를 둘 수 있음)
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

	if (OnAmmoDelegate.IsBound())
	{
		OnAmmoDelegate.Broadcast(CurrentAmmoInClip);
	}

	UE_LOG(LogTemp, Log, TEXT("Reload Complete!"));
}

bool ARangedWeapon::CanAttack()
{
	bool bBaseCan = Super::CanAttack();
	if (!bBaseCan) return false;

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

	return true;
}

void ARangedWeapon::ConsumeAmmo()
{
	if (CurrentAmmoInClip > 0) {
		CurrentAmmoInClip--;

		if (OnAmmoDelegate.IsBound()) {
			OnAmmoDelegate.Broadcast(CurrentAmmoInClip);
		}
	}
}

bool ARangedWeapon::GetCrosshairTarget(FVector& OutHitLocation)
{
	APlayerController* PC = Cast<APlayerController>(GetInstigatorController());
	if (!PC) {
		UE_LOG(LogTemp, Warning, TEXT("CrossHairError"));
		return false;
	}

	// 화면 크기 가져오기
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	// 화면 정중앙 좌표
	FVector2D ScreenLocation(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

	// 2D 좌표를 3D 월드 좌표와 방향으로 변환
	FVector WorldLocation, WorldDirection;
	bool bSuccess = PC->DeprojectScreenPositionToWorld(
		ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection
	);

	if (bSuccess) {
		// 카메라에서 레이저 쏘기
		FVector Start = PC->PlayerCameraManager->GetCameraLocation();;
		FVector End = Start + (WorldDirection * AttackRange);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this); // 무기는 무시
		QueryParams.AddIgnoredActor(GetOwner());

		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			QueryParams
		);

		if (HitResult.bBlockingHit) {
			OutHitLocation = HitResult.ImpactPoint;
			return true;
		}
		else {
			OutHitLocation = End; // 최대 사거리
			return true;
		}
	}
	return false;
}

