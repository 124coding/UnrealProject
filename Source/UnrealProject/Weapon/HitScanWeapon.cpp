// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" // 디버그 선 그리기
#include "../HitInterface.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AHitScanWeapon::OnAttack()
{
	Super::OnAttack();

	// 주인이 없다면 중단
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	AController* OwnerController = OwnerPawn->GetController();
	if (OwnerController == nullptr) return;

	// 논리적인 발사(카메라 위치)
	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation); // 카메라의 위치와 방향을 가져옴.

	FVector ShotDirection = Rotation.Vector();

	// 시작점: 카메라 위치
	// 끝점: 카메라 위치 + (방향 * 사거리)
	FVector TraceEnd = Location + (ShotDirection * AttackRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 무기는 무시
	QueryParams.AddIgnoredActor(OwnerPawn); // 쏘는 사람 무시

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Location,
		TraceEnd,
		ECollisionChannel::ECC_Visibility,
		QueryParams
	);

	// 궤적 이펙트의 목표 지점 (맞았으면 맞은 곳, 안 맞았으면 허공의 끝점)
	FVector BeamEndPoint = TraceEnd;

	if (bHit) {
		UE_LOG(LogTemp, Log, TEXT("HitScanWeapon Hit"));
		BeamEndPoint = HitResult.Location;

		// 데미지 적용
		AActor* HitActor = HitResult.GetActor();
		
		if (HitActor) {
			// 데미지를 받는 주체에게 데미지를 받았다고 메시지를 보냄.
			UE_LOG(LogTemp, Warning, TEXT("HIT: %s"), *HitActor->GetName());
			UGameplayStatics::ApplyDamage(
				HitActor,
				Damage,
				OwnerController,
				this,
				UDamageType::StaticClass()
			);

			if (HitActor->GetClass()->ImplementsInterface(UHitInterface::StaticClass())) {
				IHitInterface::Execute_GetHit(HitActor, HitResult.ImpactPoint);
			}
		}

		if (ImpactParticles) {
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				HitResult.Location,
				HitResult.ImpactNormal.Rotation() // 벽의 각도에 맞춰 이펙트 회전
			);
		}

		if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);

		DrawDebugLine(
			GetWorld(),
			Location, // 총구 위치(또는 Location)에서 시작
			BeamEndPoint,       // 계산된 끝점까지
			FColor::Red,        // 색상
			false,              // 영구 표시 X
			3.0f,               // 3초 동안 보임
			0,
			0.5f                // 선 두께
		);

		if (BeamParticles) {
			// 이펙트를 총구에 부착해서 생성
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAttached(
				BeamParticles,
				WeaponMesh,
				MuzzleSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget
			);

			if (Beam)
			{
				Beam->SetVectorParameter(BeamTargetParamName, BeamEndPoint);
			}
		}

	}
}
