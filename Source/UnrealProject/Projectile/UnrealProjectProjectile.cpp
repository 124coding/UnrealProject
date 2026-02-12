// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "../Component/ObjectPoolComponent.h"
#include "../UnrealProject.h"
#include "Kismet/GameplayStatics.h"

AUnrealProjectProjectile::AUnrealProjectProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
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


	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	LifeSpanTime = 5.0f;
}

void AUnrealProjectProjectile::DealDamage(AActor* HitActor)
{
	// hitActor에게 데미지를 받았다고 알림
	// 광역 데미지는 추후 자식 클래스에서 ApplyRadialDamage로 수정
	UGameplayStatics::ApplyDamage(
		HitActor,
		BaseDamage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);
}

void AUnrealProjectProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor && OtherActor != this)
	{
		UE_LOG(LogTemp, Log, TEXT("hit Actor: %s"), *OtherActor->GetName());
		DealDamage(OtherActor);

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
		ProjectileMovement->ProjectileGravityScale = 1.0f;
		ProjectileMovement->Activate();
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
		CollisionComp->SetSimulatePhysics(false);
	}

	if (MeshComp)
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector); // 관성 제거
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

void AUnrealProjectProjectile::LaunchTowards(FVector StartLoc, AActor* TargetActor)
{
	if (!TargetActor || !ProjectileMovement) return;

	// 기본 직사
	FVector TargetLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);

	FVector Direction = (TargetLoc - StartLoc).GetSafeNormal();
	ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
}
