// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "../Component/ObjectPoolComponent.h"
#include "../UnrealProject.h"
#include "Kismet/GameplayStatics.h"
#include "../Interface/HitInterface.h"
#include "../Interface/Interactable.h"
#include "../Character/UnrealProjectPlayerController.h"
#include "Engine/OverlapResult.h"
#include "Particles/ParticleSystemComponent.h"

AUnrealProjectProjectile::AUnrealProjectProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetSimulatePhysics(false);
	// 이 부분은 에디터에서 직접 고르는게 필요 (Player용인지 Enemy용인지)
	// CollisionComp->BodyInstance.SetCollisionProfileName("EnemyProjectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AUnrealProjectProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->SetSimulatePhysics(false);

	FlightAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("FlightAudioComp"));
	FlightAudioComp->SetupAttachment(RootComponent);

	FlightAudioComp->bAutoActivate = false;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

void AUnrealProjectProjectile::DealDamage(const FHitResult& HitResult)
{
	// 단일 타겟 직사
	if (DamageMethod == EDamageMethod::SingleTarget) {
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;

		// hitActor에게 데미지를 받았다고 알림
		UGameplayStatics::ApplyDamage(
			HitActor,
			BaseDamage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		if (!HitActor->Implements<UHitInterface>()) return;

		IHitInterface::Execute_GetHit(HitActor, HitResult.ImpactPoint);

		if (!HitActor->Implements<UInteractable>()) return;

		FText FeedbackText = IInteractable::Execute_GetFeedbackText(HitActor);

		if (FeedbackText.IsEmpty()) return;

		if (APawn* Player = Cast<APawn>(GetInstigator()))
		{
			if (AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(Player->GetController()))
			{
				PC->ShowFeedback(FeedbackText, IInteractable::Execute_GetFeedbackType(HitActor));
			}
		}
	}
	// 광역 폭발 데미지
	else if (DamageMethod == EDamageMethod::RadialDamage) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// 폭발의 중심점
		FVector Epicenter = HitResult.ImpactPoint;
	
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(),
			BaseDamage,
			MinimumDamage,
			Epicenter,
			InnerRadius,
			ExplosionRadius,
			1.0f,
			UDamageType::StaticClass(),
			IgnoredActors,
			this,
			GetInstigatorController()
		);

		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
		}

		//// 안쪽 반경 (100% 데미지 구간) - 빨간색 구체
		//DrawDebugSphere(GetWorld(), Epicenter, InnerRadius, 24, FColor::Red, false, 2.0f, 0, 2.0f);

		//// 바깥쪽 반경 (데미지 감소 구간) - 노란색 구체
		//DrawDebugSphere(GetWorld(), Epicenter, ExplosionRadius, 24, FColor::Yellow, false, 2.0f, 0, 1.0f);

		if (ExplosionParticle) {
			UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ExplosionParticle,
				Epicenter,
				FRotator::ZeroRotator
			);

			if (ParticleComp) {
				ParticleComp->SetVectorParameter(FName("ExplosionRadius"), FVector(ExplosionRadius, ExplosionRadius, ExplosionRadius));
			}
		}

		APawn* OwnerPawn = Cast<APawn>(GetInstigator());
		if (!OwnerPawn) return;

		AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(OwnerPawn->GetController());
		if (!PC) return;

		TArray<FOverlapResult> OverlapResults;
		FCollisionShape SphereCollision = FCollisionShape::MakeSphere(ExplosionRadius);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(OwnerPawn);

		bool bOverlapped = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			Epicenter,
			FQuat::Identity,
			ECC_PlayerProjectile,
			SphereCollision
		);

		if (bOverlapped)
		{

			// 맞은 액터들 중 Hit, Interact Interface가 있는 액터들은 그 코드를 수행
			for (const FOverlapResult& Overlap : OverlapResults)
			{
				AActor* CaughtActor = Overlap.GetActor();

				if (!CaughtActor || CaughtActor == this || IgnoredActors.Contains(CaughtActor)) continue;

				if (!CaughtActor->Implements<UHitInterface>()) continue;

				IHitInterface::Execute_GetHit(CaughtActor, CaughtActor->GetActorLocation());

				if (!CaughtActor->Implements<UInteractable>()) continue;

				FText FeedbackText = IInteractable::Execute_GetFeedbackText(CaughtActor);
				if (FeedbackText.IsEmpty() || !PC) continue;

				PC->ShowFeedback(FeedbackText, IInteractable::Execute_GetFeedbackType(CaughtActor));
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Radial Damage Applied at: %s"), *Epicenter.ToString());
	}
}

void AUnrealProjectProjectile::TimeOutExplode()
{
	// 현재 위치를 폭심지로
	FHitResult DummyHit;
	DummyHit.ImpactPoint = GetActorLocation();

	DealDamage(DummyHit);

	UE_LOG(LogTemp, Warning, TEXT("Explosion"));

	Deactivate();

	// 타이머 초기화
	GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandle);
}

void AUnrealProjectProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bExplodeOnTimer) return;

	if (FlightAudioComp && FlightAudioComp->IsPlaying())
	{
		FlightAudioComp->Stop();
	}

	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
	{
		DealDamage(Hit);

		Deactivate();
	}
}

void AUnrealProjectProjectile::OnPoolSpawned_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// 콜리전 다시 켜기
	if (CollisionComp) {
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 발사체 이동 컴포넌트 초기화
	if (ProjectileMovement) {
		ProjectileMovement->SetUpdatedComponent(CollisionComp);
		// ProjectileMovement->ProjectileGravityScale = 1.0f;
		ProjectileMovement->Activate();
	}

	if (FlightAudioComp && FlightAudioComp->Sound)
	{
		FlightAudioComp->Play();
	}

	// 수명 타이머
	GetWorld()->GetTimerManager().SetTimer(
		LifeSpanTimer,
		this,
		&AUnrealProjectProjectile::Deactivate,
		LifeSpanTime,
		false
	);
}

void AUnrealProjectProjectile::OnPoolReturned_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimer);
	
	if (ProjectileMovement) {
		ProjectileMovement->StopMovementImmediately(); // 가속도 및 물리력 정지
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		ProjectileMovement->Deactivate();
	}

	if (CollisionComp) {
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void AUnrealProjectProjectile::SetOwningPool_Implementation(UObjectPoolComponent* NewPool)
{
	this->OwningPoolComponent = NewPool;
}

void AUnrealProjectProjectile::Deactivate()
{
	if (GetInstigator()) {
		SetInstigator(nullptr);
	}

	// 풀에게 돌려보내달라고 요청
	if (OwningPoolComponent)
	{
		OwningPoolComponent->ReturnToPool(this);
	}
	else
	{
		// 만약 풀 없이 생성된 경우라면 그냥 파괴
		Destroy();
	}
}

void AUnrealProjectProjectile::Launch(FVector ShootDirection, float SpeedOverride)
{
	if (!ProjectileMovement) return;

	if (!GetInstigator()) {
		if (AActor* Weapon = GetOwner()) {
			SetInstigator(Cast<APawn>(Weapon->GetOwner()));
		}
	}

	// 오버라이드 값이 들어오면 본인의 속도를 사용하고 아니면 자기 속도
	float FinalSpeed = (SpeedOverride > 0.0f) ? SpeedOverride : ProjectileMovement->InitialSpeed;

	// 물리적 움직임 강제 정지
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->SetUpdatedComponent(RootComponent);

	// 정규화하여 방향만 남기고 속력 곱하기
	ProjectileMovement->Velocity = ShootDirection.GetSafeNormal() * FinalSpeed;

	// 강제로 즉시 업데이트
	ProjectileMovement->UpdateComponentVelocity();

	// 완전 재가동
	ProjectileMovement->Activate(true);

	// 총알이 날아가는 방향을 바라보게 회전
	SetActorRotation(ShootDirection.Rotation());

	// 시한폭탄 모드라면 타이머 시작
	if (bExplodeOnTimer)
	{
		ProjectileMovement->ProjectileGravityScale = 1.0f; // 투척 무기기에 중력 살리기

		GetWorld()->GetTimerManager().SetTimer(
			ExplosionTimerHandle,
			this,
			&AUnrealProjectProjectile::TimeOutExplode,
			ExplosionDelay,                           
			false                                     
		);
	}
}

void AUnrealProjectProjectile::LaunchTowards(FVector StartLoc, AActor* TargetActor)
{
	if (!TargetActor || !ProjectileMovement) return;

	// 기본 직사
	FVector TargetLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);

	FVector Direction = (TargetLoc - StartLoc).GetSafeNormal();
	
	Launch(Direction);
}

void AUnrealProjectProjectile::InitProjectile(float tDamage, EDamageMethod tDamageMethod, float tExplosionRadius, float tMinimumDamage, float tInnerRadius, bool tbExplodeOnTimer, float tExplosionDelay, float tKnockbackPower)
{
	BaseDamage = tDamage;
	DamageMethod = tDamageMethod;
	ExplosionRadius = tExplosionRadius;
	MinimumDamage = tMinimumDamage;
	InnerRadius = tInnerRadius;
	bExplodeOnTimer = tbExplodeOnTimer;
	ExplosionDelay = tExplosionDelay;
	KnockbackPower = tKnockbackPower;
}
