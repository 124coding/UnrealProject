// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "../UnrealProject.h"
#include "../Character/UnrealProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "RangedWeapon.h"
#include "../Component/CombatComponent.h"
#include "../Component/InteractionComponent.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	TArray<USceneComponent*> LocalChildren;
	WeaponMesh->GetChildrenComponents(true, LocalChildren);

	// 몸체 물리 바디
	WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody);

	// WeaponMesh의 자식들 모두 CollisionChannel 변경
	for (USceneComponent* Child : LocalChildren)
	{
		UPrimitiveComponent* PrimitiveChild = Cast<UPrimitiveComponent>(Child);
		if (PrimitiveChild)
		{
			PrimitiveChild->SetCollisionObjectType(ECC_PhysicsBody);
		}
	}
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	this->SetWeaponState(EWeaponState::Dropped);

	InitWeaponData();
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::Attack() {
	if (!CanAttack()) return;

	OnAttack();
}

bool ABaseWeapon::CanAttack()
{
	if (CurrentState == EWeaponState::Dropped) return false;

	// 무기를 소유한 플레이어가 없으면 실행 불가
	if (!GetOwner()) return false;

	// 시간 계산해서 공격 속도 체크
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < CurrentWeaponStat.AttackRate) {
		UE_LOG(LogTemp, Log, TEXT("Cant Attack So Fast"));
		return false;
	}

	return true;

}

void ABaseWeapon::OnAttack()
{
	if (CurrentWeaponStat.FireMode == EWeaponFireMode::Single)
	{
		// 단발 (Single)
		ExecuteFire();
	}
	else if (CurrentWeaponStat.FireMode == EWeaponFireMode::Auto)
	{
		// 연사 (Auto)
		ExecuteFire();
		GetWorldTimerManager().SetTimer(
			FireTimerHandle, this, &ABaseWeapon::ExecuteFire, CurrentWeaponStat.AttackRate, true
		);
	}
	else if (CurrentWeaponStat.FireMode == EWeaponFireMode::Burst)
	{
		// 점사 (Burst)
		CurrentBurstCount = 0;
		HandleBurstFire();
	}
}

void ABaseWeapon::StopAttack() {

	if (CurrentWeaponStat.FireMode == EWeaponFireMode::Auto)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
}

void ABaseWeapon::ExecuteFire()
{
	LastAttackTime = GetWorld()->GetTimeSeconds();

	// 무기를 소유한 플레이어가 없으면 실행 불가
	AUnrealProjectCharacter* OwnerCharacter = Cast<AUnrealProjectCharacter>(GetOwner());
	if (OwnerCharacter == nullptr || OwnerCharacter->GetController() == nullptr)
	{
		return;
	}

	if (OwnerCharacter->IsDead()) StopAttack();

	// 공격 소리 존재 시 소리 출력
	if (CurrentWeaponStat.AttackSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CurrentWeaponStat.AttackSound, GetActorLocation());

		// 소리를 적들이 들을 수 있게 함
		MakeNoise(1.0f, OwnerCharacter, GetActorLocation());
	}

	if (CurrentWeaponStat.FireAnimation != nullptr) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(CurrentWeaponStat.FireAnimation, 1.f);
		}
	}
}

void ABaseWeapon::HandleBurstFire()
{
	if (CurrentBurstCount >= CurrentWeaponStat.MaxBurstCount) return;

	// 한번 공격
	ExecuteFire();
	CurrentBurstCount++;

	// 남은 공격 횟수가 있다면 다시 호출

	if (CurrentBurstCount < CurrentWeaponStat.MaxBurstCount)
	{
		GetWorldTimerManager().SetTimer(
			FireTimerHandle, this, &ABaseWeapon::HandleBurstFire, CurrentWeaponStat.BurstFireRate, false
		);
	}
}

void ABaseWeapon::Interact_Implementation(AActor* InstigatorActor)
{
	if (this->CurrentState == EWeaponState::Equipped) return;

	AUnrealProjectCharacter* OwnerCharacter = Cast<AUnrealProjectCharacter>(InstigatorActor);
	if (OwnerCharacter == nullptr || OwnerCharacter->GetController() == nullptr)
	{
		return;
	}

	if (UCombatComponent* CombatComp = OwnerCharacter->CombatComponent) {
		CombatComp->PickupWeapon(this);
	}
}

void ABaseWeapon::SetWeaponState(EWeaponState NewState)
{
	CurrentState = NewState;

	if (!WeaponMesh) return;

	switch (CurrentState) {
	case EWeaponState::Equipped:

		UE_LOG(LogTemp, Log, TEXT("Weapon Equipped"));

		// 물리 끄기
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);

		WeaponMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		WeaponMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		if (RootComponent)
		{
			WeaponMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
		WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);

		// 몸체 WorldStatic
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (AreaSphere) AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 반동 제어를 위한 Tick 켜기
		if (Cast<ARangedWeapon>(this)) {
			SetActorTickEnabled(true);
		}

		break;
	
	case EWeaponState::Dropped:

		UE_LOG(LogTemp, Log, TEXT("Weapon Dropped"));

		// 물리 켜기
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block); // 기본적으로 다 막음
		WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // Pawn 통과
		WeaponMesh->SetCollisionResponseToChannel(ECC_Enemy, ECR_Ignore); // 적 통과
		WeaponMesh->SetCollisionResponseToChannel(ECC_Player, ECR_Ignore); // 플레이어 통과
		WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 카메라는 통과

		// Interact 채널에 대해 Block하라고 명령, 상호작용 켜기
		WeaponMesh->SetCollisionResponseToChannel(ECC_Interact, ECR_Block);
		if (AreaSphere) AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Tick 끄기
		SetActorTickEnabled(false);
		break;
	}
}

void ABaseWeapon::InitWeaponData()
{
	if (!WeaponDataHandle.IsNull())
	{
		FBaseWeaponStatRow* RowData = WeaponDataHandle.GetRow<FBaseWeaponStatRow>(TEXT("WeaponDataLookup"));

		if (RowData)
		{
			CurrentWeaponStat = *RowData;
		}
	}
}

