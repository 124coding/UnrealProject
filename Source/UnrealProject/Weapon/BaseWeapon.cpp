// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "../UnrealProject.h"
#include "../Character/UnrealProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "RangedWeapon.h"
#include "../Component/CombatComponent.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	this->SetWeaponState(EWeaponState::Dropped);
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::Attack() {
	if (!CanAttack()) return;

	// 무기를 소유한 플레이어가 없으면 실행 불가
	AUnrealProjectCharacter* OwnerCharacter = Cast<AUnrealProjectCharacter>(GetOwner());
	if (OwnerCharacter == nullptr || OwnerCharacter->GetController() == nullptr)
	{
		return;
	}

	// 공격 소리 존재 시 소리 출력
	if (AttackSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());

		// 소리를 적들이 들을 수 있게 함
		MakeNoise(1.0f, OwnerCharacter, GetActorLocation());
	}

	if (FireAnimation != nullptr) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	OnAttack();
}

bool ABaseWeapon::CanAttack()
{
	if (CurrentState == EWeaponState::Dropped) return false;

	// 무기를 소유한 플레이어가 없으면 실행 불가
	if (!GetOwner()) return false;

	return true;

}

void ABaseWeapon::OnAttack()
{
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

	switch (CurrentState) {
	case EWeaponState::Equipped:

		if (!WeaponMesh) return;
		UE_LOG(LogTemp, Log, TEXT("Weapon Equipped"));

		// 물리 끄기
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		WeaponMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
		WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);

		// 몸체 WorldStatic
		WeaponMesh->SetCollisionObjectType(ECC_WorldStatic);

		if (AreaSphere) AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 반동 제어를 위한 Tick 켜기
		if (Cast<ARangedWeapon>(this)) {
			SetActorTickEnabled(true);
		}

		break;
	
	case EWeaponState::Dropped:
	
		if (!WeaponMesh) return;

		UE_LOG(LogTemp, Log, TEXT("Weapon Dropped"));

		// 물리 켜기
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// 몸체 물리 바디
		WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody);

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

