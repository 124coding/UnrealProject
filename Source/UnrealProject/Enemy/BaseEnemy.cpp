// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "EnemyAIController.h"
#include "../UnrealProject.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Component/ObjectPoolComponent.h"
#include "../UnrealProjectGameMode.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemy::OnEnemyOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemy::OnEnemyOverlapEnd);

	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_EnemyProjectile, ECR_Ignore);

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
	
	if (GetMesh())
	{
		InitialMeshTransform = GetMesh()->GetRelativeTransform();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// GetMesh()->SetCollisionProfileName(TEXT("Enemy"));
		GetMesh()->SetSimulatePhysics(false);
	}

	// Tick 대신 0.1초마다 밀어내는 힘을 계산하는 타이머
	FTimerHandle SeparationTimer;
	GetWorldTimerManager().SetTimer(SeparationTimer, this, &ABaseEnemy::CalculateSeparation, 0.1f, true);
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentRepulsionForce.IsNearlyZero())
	{
		AddActorWorldOffset(CurrentRepulsionForce * DeltaTime, true);
	}
}

bool ABaseEnemy::CanAttack() {
	if (CurrentState == EEnemyState::EES_Normal) {
		return true;
	}

	return false;
}

void ABaseEnemy::OnAttack() {
}

void ABaseEnemy::Attack() {
	if (!CanAttack()) return;
	UE_LOG(LogTemp, Log, TEXT("Enemy Attack"));

	OnAttack();
}

bool ABaseEnemy::IsAttacking() const
{
	return CurrentState == EEnemyState::EES_Attacking;
}

void ABaseEnemy::SetCommandTarget(AActor* NewTarget)
{
	AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());

	if (AIC && AIC->GetBlackboardComponent()) {
		AIC->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), NewTarget);
	}
}

void ABaseEnemy::PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageAmount)
{
	// 트레이스 시작 위치
	FVector Start = GetMesh()->GetSocketLocation(SocketName);
	FVector End = Start + (GetActorForwardVector() * 20.0f);

	// 충돌 대상 설정 (Player만 감지하도록)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Player));

	// 자기 자신은 무시
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

	if (CurrentState == EEnemyState::EES_Stunned) {
		if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController())) {
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->ResumeLogic("Hit Reaction");
			}
		}
	}
	else if (CurrentState == EEnemyState::EES_Attacking) {
		/* 다음 공격까지의 딜레이 주는 것도 가능*/
	}

	UE_LOG(LogTemp, Warning, TEXT("EES_Normal"));
	CurrentState = EEnemyState::EES_Normal;
}

void ABaseEnemy::PlayDirectionalHitReact(const FVector& ImpactPoint)
{
	// 적의 정면 벡터
	FVector Forward = GetActorForwardVector();

	// 피격 지점 벡터(적의 위치 -> 때린 위치)
	/*FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z);
	FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();*/

	FVector EnemyLoc = GetActorLocation();
	FVector ToHit = (ImpactPoint - EnemyLoc);
	ToHit.Z = 0.f;  // Z축 간섭 제거
	ToHit = ToHit.GetSafeNormal();

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

	if (Theta >= -70.0f && Theta <= 70.0f) {
		MontageToPlay = HitReactMontage_Front;
		UE_LOG(LogTemp, Warning, TEXT("Front Hit"));
	}
	else if (Theta >= -110.0f && Theta < -70.0f) {
		MontageToPlay = HitReactMontage_Left;
		UE_LOG(LogTemp, Warning, TEXT("Left Hit"));
	}
	else if (Theta > 70.0f && Theta <= 110.0f) {
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

		if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->ResumeLogic("Hit Reaction"); // 즉시 재가동
			}
		}
	}
}

void ABaseEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Hit"));

	if(CurrentState == EEnemyState::EES_Dead) return;

	CurrentState = EEnemyState::EES_Stunned;

	if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController())) {
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->PauseLogic("Hit Reaction");
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

void ABaseEnemy::OnPoolSpawned_Implementation()
{
	GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

	// Actor 기본 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// 물리 끄기
	if (GetMesh()) {
		GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
		GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		GetMesh()->SetRelativeTransform(InitialMeshTransform);
		GetMesh()->bPauseAnims = false;

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance()) {
			AnimInstance->StopAllMontages(0.0f);
		}

		if (GetCapsuleComponent()) {
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = 1.0f; // 중력 복구
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->Activate(); // 컴포넌트 활성화
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	CurrentState = EEnemyState::EES_Normal;

	if (AttributeComponent)
	{
		AttributeComponent->InitializeStats();
	}

	// AI 재가동
	AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());
	if (AIC)
	{
		if (AIC->BehaviorTreeAsset) {
			AIC->RunBehaviorTree(AIC->BehaviorTreeAsset);
		}
		else {
			AIC->GetBrainComponent()->RestartLogic(); // 비헤이비어 트리 재시작
		}
	}
}

void ABaseEnemy::OnPoolReturned_Implementation()
{
	// 걸려있는 모든 타이머 취소 (공격 쿨타임, 사망 타이머 등)
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// 물리 끄기
	/*if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}*/

	if (GetCapsuleComponent()) {
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// AI 정지 시키기
	AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());
	if (AIC)
	{
		// BrainComponent(비헤이비어 트리)가 있다면 스톱
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic("ReturnedToPool");
		}
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately(); // 이동 정지
		GetCharacterMovement()->GravityScale = 0.0f; // 중력 끄기 (둥둥 뜨게)
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None); // 이동 모드 없음
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ABaseEnemy::SetOwningPool_Implementation(UObjectPoolComponent* NewPool)
{
	this->OwningPoolComponent = NewPool;
}

void ABaseEnemy::HandleDeath(AActor* VictimActor, AActor* KillerActor)
{
	if (CurrentState == EEnemyState::EES_Dead) return;

	// 상태 변경
	CurrentState = EEnemyState::EES_Dead;

	// 동작 정지
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// 애니메이션 로직 중단
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
	}

	// 충돌 끔
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 래그돌 실행
	/*GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);*/

	// 사망 애니메이션
	float DeathAnimDuration = PlayAnimMontage(DeathMontage);
	if (DeathAnimDuration > 0.f)
	{
		// 애니메이션이 끝나기 직전에 애니메이션을 굳히기
		GetWorldTimerManager().SetTimer(AnimFreezeTimerHandle, this, &ABaseEnemy::FreezeAnimation, DeathAnimDuration - 0.1f, false);
	}

	GetWorldTimerManager().SetTimer(ReturnTimerHandle, this, &ABaseEnemy::Deactivate, 5.0f, false);

	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		// BrainComponent(비헤이비어 트리)가 있다면 스톱
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic("Death");
		}
	}

	if (AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode()))
	{
		// 적이 죽었으므로 카운트 감소
		GM->ActiveEnemyCount = FMath::Max(0, GM->ActiveEnemyCount - 1);
	}

	// 5초 뒤에 삭제
	//SetLifeSpan(5.0f);
}

void ABaseEnemy::Deactivate()
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

void ABaseEnemy::FreezeAnimation()
{
	if (GetMesh())
	{
		GetMesh()->bPauseAnims = true;
	}
}

void ABaseEnemy::OnEnemyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABaseEnemy* OverlappedEnemy = Cast<ABaseEnemy>(OtherActor);
	if (OverlappedEnemy && OverlappedEnemy != this)
	{
		CachedNeighbors.AddUnique(OverlappedEnemy); // 명단에 추가
	}
}

void ABaseEnemy::OnEnemyOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABaseEnemy* OverlappedEnemy = Cast<ABaseEnemy>(OtherActor);
	if (OverlappedEnemy)
	{
		CachedNeighbors.Remove(OverlappedEnemy); // 명단에서 삭제
	}
}

void ABaseEnemy::CalculateSeparation()
{
	CurrentRepulsionForce = FVector::ZeroVector;

	if (CachedNeighbors.Num() == 0) return;

	for (ABaseEnemy* Neighbor : CachedNeighbors) {
		// 죽은 적은 계산 X
		if (Neighbor && Neighbor->CurrentState != EEnemyState::EES_Dead) {
			FVector PushDirection = GetActorLocation() - Neighbor->GetActorLocation();
			PushDirection.Z = 0.0f;

			float Distance = PushDirection.Size();
			if (Distance > 1.0f) {
				CurrentRepulsionForce += (PushDirection.GetSafeNormal() * (150.0f / Distance));
			}
		}
	}

	// 여러 마리의 힘이 동시에 들어오는걸 방지
	CurrentRepulsionForce = CurrentRepulsionForce.GetClampedToMaxSize(300.0f);
}
