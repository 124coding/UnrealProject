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
#include "../Component/VocalComponent.h"
#include "../Component/FootstepComponent.h"
#include "../UnrealProjectGameMode.h"
#include "../Weapon/BaseWeapon.h"
#include "../Projectile/UnrealProjectProjectile.h"
#include "Engine/DamageEvents.h"
#include "../UI/DamageTextActor.h"
#include "../PoolState.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemy::OnEnemyOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemy::OnEnemyOverlapEnd);

	VocalComponent = CreateDefaultSubobject<UVocalComponent>(TEXT("VocalComp"));
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComp"));

	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_EnemyProjectile, ECR_Ignore);

}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitEnemyData();

	if (AttributeComponent) {
		AttributeComponent->OnDeath.AddDynamic(this, &ABaseEnemy::HandleDeath);
		AttributeComponent->OnKnockback.AddDynamic(this, &ABaseEnemy::HandleKnockback);
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
		static const FName TargetActorKey = TEXT("TargetActor");

		if (AIC->GetBlackboardComponent()->GetValueAsObject(TargetActorKey) != NewTarget) {
			AIC->GetBlackboardComponent()->SetValueAsObject(TargetActorKey, NewTarget);
		}

		if (VocalComponent)
		{
			if (NewTarget != nullptr)
			{
				VocalComponent->SetVocalState(EVocalState::EVS_Sprinting);
			}
			else
			{
				VocalComponent->SetVocalState(EVocalState::EVS_Idle);
			}
		}
	}
}

void ABaseEnemy::PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageMultiplier)
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
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	if (bResult) {
		AActor* HitActor = OutHit.GetActor();

		// 플레이어인지 확인 후 데미지
		if (HitActor) {
			UGameplayStatics::ApplyDamage(
				HitActor,
				CurrentEnemyStat.Damage * DamageMultiplier,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}
	}
}

void ABaseEnemy::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}

	if (CurrentState == EEnemyState::EES_Stunned) {
		if (GetCharacterMovement()->IsFalling())
		{
			// AI Resume도 하지 않고, Normal로도 바꾸지 않습니다. (Landed 함수가 나중에 처리할 겁니다)
			return;
		}

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

void ABaseEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 바닥에 떨어졌는데 아직 스턴 상태라면
	if (CurrentState == EEnemyState::EES_Stunned)
	{
		// 혹시라도 피격 애니메이션이 아주 길어서 아직 재생 중인지 체크
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
		{
			// 몽타주도 끝났고, 땅에도 닿았으니 완벽하게 복구!
			if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController())) {
				if (UBrainComponent* Brain = AIController->GetBrainComponent())
				{
					GetCharacterMovement()->AirControl = 0.05f;
					Brain->ResumeLogic("Hit Reaction");
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Landed & EES_Normal"));
			CurrentState = EEnemyState::EES_Normal;
		}
	}
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
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Front;
		UE_LOG(LogTemp, Warning, TEXT("Front Hit"));
	}
	else if (Theta >= -110.0f && Theta < -70.0f) {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Left;
		UE_LOG(LogTemp, Warning, TEXT("Left Hit"));
	}
	else if (Theta > 70.0f && Theta <= 110.0f) {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Right;
		UE_LOG(LogTemp, Warning, TEXT("Right Hit"));
	}
	else {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Back;
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
		// 임시로 0.5초 동안 몸이 굳은(Stun) 상태를 유지하다가 땅에 떨어지면 풀리도록 세팅
		FTimerHandle FakeHitReactTimer;
		GetWorldTimerManager().SetTimer(FakeHitReactTimer, FTimerDelegate::CreateLambda([this]()
			{
				if (!GetCharacterMovement()->IsFalling())
				{
					CurrentState = EEnemyState::EES_Normal;
					if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController()))
					{
						if (UBrainComponent* Brain = AIController->GetBrainComponent())
						{
							Brain->ResumeLogic("Hit Reaction");
						}
					}
				}
			}), 0.5f, false);
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

	if (VocalComponent) {
		VocalComponent->PlayVocalEvent(EVocalEvent::EVE_Damaged);
	}

	PlayDirectionalHitReact(ImpactPoint);

	/*UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABaseEnemy::OnMontageEnded);
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseEnemy::OnMontageEnded);
	}*/
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	GetCharacterMovement()->StopMovementImmediately();

	// 타겟 지정
	if (EventInstigator && EventInstigator->GetPawn())
	{
		SetCommandTarget(EventInstigator->GetPawn());
	}
	else if (DamageCauser)
	{
		// 혹시라도 Instigator가 없는 환경 데미지 등일 경우를 대비
		SetCommandTarget(DamageCauser);
	}

	APoolState* GS = GetWorld()->GetGameState<APoolState>();
	if (GS && GS->DamageTextPool) {
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);

		float RandomX = FMath::RandRange(-20.f, 20.f);
		float RandomY = FMath::RandRange(-20.f, 20.f);
		float RandomZ = FMath::RandRange(-10.f, 10.f);

		SpawnLocation += FVector(RandomX, RandomY, RandomZ);

		AActor* PooledActor = GS->DamageTextPool->SpawnFromPool(SpawnLocation, FRotator::ZeroRotator);

		ADamageTextActor* DamageText = Cast<ADamageTextActor>(PooledActor);
		if (DamageText) {
			DamageText->ShowDamage(SpawnLocation, ActualDamage);
		}
	}
	

	return ActualDamage;
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

	if (VocalComponent) {
		VocalComponent->PlayVocalEvent(EVocalEvent::EVE_Spawn);
		VocalComponent->SetVocalState(EVocalState::EVS_Idle);
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

	if (VocalComponent) {
		VocalComponent->SetVocalState(EVocalState::EVS_None);
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

	if (VocalComponent) {
		VocalComponent->PlayVocalEvent(EVocalEvent::EVE_Death);
	}

	// 사망 애니메이션
	float DeathAnimDuration = PlayAnimMontage(CurrentEnemyStat.DeathMontage);
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

void ABaseEnemy::HandleKnockback(FVector PushDirection, float Force)
{
	if (CurrentState == EEnemyState::EES_Dead) return;

	FVector FinalKnockback = PushDirection * Force;
	
	// 수직 부양력 보정
	float VerticalLiftMultiplier = 0.5f;
	FinalKnockback.Z = Force * VerticalLiftMultiplier;

	// 마찰력 씹힘 방지 및 공중 제어 불가
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->AirControl = 0.0f;

	this->LaunchCharacter(FinalKnockback, true, true);
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

void ABaseEnemy::InitEnemyData()
{
	if (!EnemyDataHandle.IsNull())
	{
		FBaseEnemyStatRow* RowData = EnemyDataHandle.GetRow<FBaseEnemyStatRow>(TEXT("EnemyDataLookup"));

		if (RowData)
		{
			CurrentEnemyStat = *RowData;
		}
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
