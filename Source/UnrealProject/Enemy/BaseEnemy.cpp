// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"
#include "BrainComponent.h"

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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseEnemy::OnMontageEnded);
	}
	
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::Attack() {
	UE_LOG(LogTemp, Log, TEXT("Enemy Attack"));
}

bool ABaseEnemy::IsAttacking() const
{
	return CurrentState == EEnemyState::EES_Attacking;
}

void ABaseEnemy::PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageAmount)
{
	// 트레이스 시작 위치
	FVector Start = GetMesh()->GetSocketLocation(SocketName);
	FVector End = Start + (GetActorForwardVector() * 20.0f);

	// 충돌 대상 설정 (Pawn만 감지하도록)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 자기 자신은 무시 (추후에 모든 적에 대해서도 무시가 필요)
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	FHitResult OutHit;

	bool bResult = UKismetSystemLibrary::SphereTraceSingleForObjects(
		GetWorld(),
		Start,
		End,
		HalfRadiusSize,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration, // 디버그용(추후 None으로 변경)
		OutHit,
		true
	);

	if (bResult) {
		AActor* HitActor = OutHit.GetActor();

		// 플레이어인지 확인 후 데미지
		if (HitActor) {
			UGameplayStatics::ApplyDamage(
				HitActor,
				DamageAmount,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}

		/*플레이어 피격 이펙트 / 소리 재생 인터페이스 호출 필요*/
	}
}

void ABaseEnemy::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("EES_Normal"));
	CurrentState = EEnemyState::EES_Normal;

	if (AAIController* AIController = Cast<AAIController>(GetController())) {
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StartLogic();
		}
	}

	/* 다음 공격까지의 딜레이 주는 것도 가능*/
}

void ABaseEnemy::PlayDirectionalHitReact(const FVector& ImpactPoint)
{
	// 적의 정면 벡터
	FVector Forward = GetActorForwardVector();

	// 피격 지점 벡터(적의 위치 -> 때린 위치)
	FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z);
	FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// 각도 구하기
	// 0도: 정면, 180/-180도: 후면, 90도: 우측, -90도: 좌측
	double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0) {
		Theta *= -1.0f;
	}

	UAnimMontage* MontageToPlay = nullptr;

	if (Theta >= -80.0f && Theta <= 80.0f) {
		MontageToPlay = HitReactMontage_Front;
		UE_LOG(LogTemp, Warning, TEXT("Front Hit"));
	}
	else if (Theta >= -100.0f && Theta < -80.0f) {
		MontageToPlay = HitReactMontage_Left;
		UE_LOG(LogTemp, Warning, TEXT("Left Hit"));
	}
	else if (Theta > 80.0f && Theta <= 100.0f) {
		MontageToPlay = HitReactMontage_Right;
		UE_LOG(LogTemp, Warning, TEXT("Right Hit"));
	}
	else {
		MontageToPlay = HitReactMontage_Back;
		UE_LOG(LogTemp, Warning, TEXT("Back Hit"));
	}

	// 몽타주 재생 시도 
	float Duration = 0.0f;

	if (MontageToPlay)
	{
		Duration = PlayAnimMontage(MontageToPlay);
	}

	if (Duration <= 0.0f)
	{
		// 바로 풀어주지 않으면 AI가 영원히 멈춤
		UE_LOG(LogTemp, Error, TEXT("Montage Play Error"));

		CurrentState = EEnemyState::EES_Normal;

		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->StartLogic(); // 즉시 재가동
			}
		}
	}
}

void ABaseEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Hit"));

	if(CurrentState == EEnemyState::EES_Dead) return;

	CurrentState = EEnemyState::EES_Stunned;

	if (AAIController* AIController = Cast<AAIController>(GetController())) {
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic("Hit Reaction");
		}
	}

	PlayDirectionalHitReact(ImpactPoint);

	/*UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABaseEnemy::OnMontageEnded);
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseEnemy::OnMontageEnded);
	}*/
}

void ABaseEnemy::HandleDeath(AActor* VictimActor, AActor* KillerActor)
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

