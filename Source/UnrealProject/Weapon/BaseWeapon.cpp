// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "../Character/UnrealProjectCharacter.h"
#include "Kismet/GameplayStatics.h"

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
	
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::Attack()
{
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

}

