// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" // 디버그 선 그리기
#include "../Interface/HitInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "../UnrealProject.h"
#include "../Character/UnrealProjectPlayerController.h"
#include "Components/DecalComponent.h"

void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < 30; ++i) {
		UParticleSystemComponent* PooledImpact = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			CurrentHitScanStat.DefaultImpactParticle,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(0.5f),
			false
		);

		if (PooledImpact) {
			PooledImpact->Deactivate();
			ImpactPool.Add(PooledImpact);
		}
	}

	if (CurrentHitScanStat.BeamParticles) {
		// 이펙트를 총구에 부착해서 생성
		PooledBeamComponent = UGameplayStatics::SpawnEmitterAttached(
			CurrentHitScanStat.BeamParticles,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);

		if (PooledBeamComponent)
		{
			PooledBeamComponent->Deactivate();
		}
	}

	DecalTimers.SetNum(30);
	for (int32 i = 0; i < 30; i++)
	{
		UDecalComponent* Decal = NewObject<UDecalComponent>(this);
		Decal->RegisterComponentWithWorld(GetWorld());
		Decal->SetVisibility(false);

		DecalPool.Add(Decal);
	}
}

void AHitScanWeapon::ExecuteFire()
{
	Super::ExecuteFire();

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
	FVector TraceEnd = Location + (ShotDirection * CurrentHitScanStat.AttackRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 무기는 무시
	QueryParams.AddIgnoredActor(OwnerPawn); // 쏘는 사람 무시
	QueryParams.bReturnPhysicalMaterial = true; // 재질가져오기

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Location,
		TraceEnd,
		ECollisionChannel::ECC_PlayerProjectile,
		QueryParams
	);

	// 궤적 이펙트의 목표 지점 (맞았으면 맞은 곳, 안 맞았으면 허공의 끝점)
	FVector BeamEndPoint = bHit ? HitResult.ImpactPoint : TraceEnd;

	FVector MuzzleLocation = FVector::ZeroVector;
	if (WeaponMesh) {
		MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}

	if (bHit) {
		UE_LOG(LogTemp, Log, TEXT("HitScanWeapon Hit"));

		// 데미지 적용
		AActor* HitActor = HitResult.GetActor();
		
		if (HitActor) {
			// 데미지를 받는 주체에게 데미지를 받았다고 메시지를 보냄.
			UE_LOG(LogTemp, Warning, TEXT("HIT: %s"), *HitActor->GetName());
			UGameplayStatics::ApplyDamage(
				HitActor,
				CurrentRangedStat.Damage,
				OwnerController,
				this,
				UDamageType::StaticClass()
			);

			if (HitActor->Implements<UHitInterface>()) {
				IHitInterface::Execute_GetHit(HitActor, HitResult.ImpactPoint);
			}

			if (HitActor->Implements<UInteractable>()) {
				FText FeedbackText = IInteractable::Execute_GetFeedbackText(HitActor);

				if (!FeedbackText.IsEmpty()) {
					if (APawn* Player = Cast<APawn>(GetOwner()))
					{
						if (AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(Player->GetController()))
						{
							PC->ShowFeedback(FeedbackText, IInteractable::Execute_GetFeedbackType(HitActor));
						}
					}
				}
			}
		}

		// 피격 이펙트
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		if (HitResult.PhysMaterial.IsValid()) // 재질 정보가 있는지 안전 검사
		{
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
		}

		UParticleSystem* SelectedParticle = CurrentHitScanStat.DefaultImpactParticle;

		if (SelectedParticle) {
			if (CurrentHitScanStat.ImpactParticleMap.Contains(SurfaceType))
			{
				SelectedParticle = CurrentHitScanStat.ImpactParticleMap[SurfaceType];
			}

			UParticleSystemComponent* Target = nullptr;
			for (auto Comp : ImpactPool)
			{
				if (!Comp->IsActive())
				{
					Target = Comp;
					break;
				}
			}

			if (Target == nullptr)
			{
				Target = ImpactPool[ImpactIndex % ImpactPool.Num()];
				ImpactIndex++;

				Target->Deactivate();
			}

			Target->SetTemplate(SelectedParticle);
			Target->SetWorldLocationAndRotation(HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			Target->Activate(true);
		}

		// 총알 흔적

		if (HitActor && !HitActor->IsA<APawn>()) {
			UMaterialInterface* DecalMaterial = nullptr;

			if (CurrentHitScanStat.DefaultDecal) {
				DecalMaterial = CurrentHitScanStat.DefaultDecal;
			}

			if (CurrentHitScanStat.DecalMap.Contains(SurfaceType))
			{
				DecalMaterial = CurrentHitScanStat.DecalMap[SurfaceType];
			}

			if (DecalMaterial && HitResult.bBlockingHit && DecalPool.Num() > 0) {

				// 가장 오래된 데칼 컴포넌트
				int32 CurrentTargetIndex = DecalIndex % DecalPool.Num();
				UDecalComponent* TargetDecal = DecalPool[CurrentTargetIndex];
				DecalIndex++;

				// 재질과 크기를 세팅
				TargetDecal->SetDecalMaterial(DecalMaterial);
				TargetDecal->DecalSize = FVector(10.f, 5.f, 5.f);

				// 맞은 위치와 각도로 이동시
				TargetDecal->SetWorldLocationAndRotation(
					HitResult.ImpactPoint,
					(-HitResult.ImpactNormal).Rotation()
				);

				TargetDecal->SetVisibility(true);

				GetWorld()->GetTimerManager().ClearTimer(DecalTimers[CurrentTargetIndex]);

				FTimerDelegate HideDelegate;
				HideDelegate.BindLambda([TargetDecal]()
					{
						if (IsValid(TargetDecal))
						{
							TargetDecal->SetVisibility(false);
						}
					});

				GetWorld()->GetTimerManager().SetTimer(
					DecalTimers[CurrentTargetIndex],
					HideDelegate,
					5.0f,
					false
				);
			}
		}
		
		if(HitActor && HitActor->IsA<APawn>()){
			AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(GetInstigatorController());
			if (PC) {
				PC->PlayHitMarkerEvent();
			}
		}

		// 피격 사운드

		USoundBase* SelectedSound = nullptr; 
		if (CurrentHitScanStat.DefaultImpactSound) {
			SelectedSound = CurrentHitScanStat.DefaultImpactSound; // 일단 기본 사운드
		}

		// 맞은 재질이 Tmap에 있다면 교체
		if (CurrentHitScanStat.ImpactSoundMap.Contains(SurfaceType))
		{
			SelectedSound = CurrentHitScanStat.ImpactSoundMap[SurfaceType];
		}

		if (SelectedSound) {
			UGameplayStatics::PlaySoundAtLocation(
				this, 
				SelectedSound, 
				HitResult.ImpactPoint
			);
		}

		DrawDebugLine(
			GetWorld(),
			MuzzleLocation, // 총구 위치(또는 Location)에서 시작
			BeamEndPoint,       // 계산된 끝점까지
			FColor::Red,        // 색상
			false,              // 영구 표시 X
			3.0f,               // 3초 동안 보임
			0,
			0.5f                // 선 두께
		);

		if (PooledBeamComponent) {
			PooledBeamComponent->SetVectorParameter(BeamTargetParamName, BeamEndPoint);

			PooledBeamComponent->Activate(true);
		}
	}
}

void AHitScanWeapon::InitWeaponData()
{
	if (!WeaponDataHandle.IsNull())
	{
		FHitScanWeaponStatRow* RowData = WeaponDataHandle.GetRow<FHitScanWeaponStatRow>(TEXT("HitScanWeaponDataLookup"));

		if (RowData)
		{
			CurrentHitScanStat = *RowData;
			CurrentRangedStat = *RowData;
			CurrentWeaponStat = *RowData;
		}
	}
}
