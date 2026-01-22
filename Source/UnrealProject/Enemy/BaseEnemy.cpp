// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeComponent) {
		AttributeComponent->OnDeath.AddDynamic(this, &ABaseEnemy::HandleDeath);
	}
	
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseEnemy::MeleeAttack() {

}

void ABaseEnemy::HandleDeath()
{
	if (CurrentState == EEnemyState::EES_Dead) return;

	// 상태 변경
	CurrentState = EEnemyState::EES_Dead;

	// 동작 정지
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// 충돌 끔
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 래그돌 실행
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	// 5초 뒤에 삭제
	SetLifeSpan(5.0f);
}

