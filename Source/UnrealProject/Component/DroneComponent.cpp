// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../UnrealProject.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "../SurvivalGameInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "AttributeComponent.h"

// Sets default values for this component's properties
UDroneComponent::UDroneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDroneComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!DroneDataHandle.IsNull())
	{
		// 엑셀에서 내가 선택한 행(Row)을 FDroneStats 타입으로 찾아오기
		FDroneStats* RowData = DroneDataHandle.GetRow<FDroneStats>(TEXT("DroneDataLookup"));

		if (RowData)
		{
			// 원본 데이터를 내 실시간 스탯(CurrentStats)에 통째로 복사(Copy)
			CurrentStats = *RowData;
		}
	}

	USurvivalGameInstance* GI = Cast<USurvivalGameInstance>(GetWorld()->GetGameInstance());
	if (GI) LoadDataFromGI(GI);

	// 런타임에 레이더 생성 및 플레이어 부착
	//DetectionSphere = NewObject<USphereComponent>(GetOwner(), TEXT("DroneAttackRadar"));
	//if (DetectionSphere) {
	//	DetectionSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	//	// 레이더 반경 설정
	//	DetectionSphere->SetSphereRadius(CurrentStats.AttackRange);
	//	
	//	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	//	DetectionSphere->SetCollisionResponseToChannel(ECC_Enemy, ECR_Overlap);

	//	// 델리게이트 연결
	//	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UDroneComponent::OnRadarBeginOverlap);
	//	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UDroneComponent::OnRadarEndOverlap);

	//	DetectionSphere->RegisterComponent();
	//}

	// 0.1초마다 주위 적을 찾는 걸로 수정
	GetWorld()->GetTimerManager().SetTimer(
		RadarScanTimerHandle,
		this,
		&UDroneComponent::PerformRadarScan,
		0.1f,
		true
	);

	// 런타임에 드론 외형 생성 및 부착
	DroneMesh = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("DroneMesh"));
	if (DroneMesh)
	{
		DroneMesh->RegisterComponent();
		// 플레이어의 Root나 특정 소켓(예: 어깨 위)에 부착
		DroneMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		DroneMesh->SetUsingAbsoluteLocation(true);
		DroneMesh->SetUsingAbsoluteRotation(true);

		if (CurrentStats.DroneMeshAsset)
		{
			DroneMesh->SetStaticMesh(CurrentStats.DroneMeshAsset);
		}

		DroneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 1초마다 패시브 로직 실행
	UpdateDroneTimers();
	// ...
	
}


// Called every frame
void UDroneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!DroneMesh || !GetOwner()) return;

	// 드론이 가야 할 원래 목표 위치
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	FRotator PlayerRotation = GetOwner()->GetActorRotation();
	FRotator YawOnlyRotation = FRotator(0.0f, PlayerRotation.Yaw, 0.0f);

	// 오프셋에 플레이어의 회전값을 곱하기
	FVector DesiredTargetLocation = PlayerLocation + YawOnlyRotation.RotateVector(HoverOffset);

	// 공격 중이 아닐때만 호버링
	if (!bIsAttacking)
	{
		// 시간에 따라 위아래로 움직이는 사인 파형 추가 (진폭 15.0f, 속도 2.0f)
		float HoverZ = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 15.0f;
		DesiredTargetLocation.Z += HoverZ;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner()); // 플레이어 몸통은 무시

	// 플레이어 -> 이상적인 목표 위치로 선을 그어 벽(WorldStatic)이 있는지 검사
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		PlayerLocation,
		DesiredTargetLocation,
		ECC_WorldStatic,
		QueryParams
	);

	FVector TargetLocation;

	if (bHit)
	{
		// 벽에 막혔다면? 목표 위치를 벽에 부딪힌 지점으로
		TargetLocation = HitResult.ImpactPoint + (HitResult.ImpactNormal * 30.0f);
	}
	else
	{
		// 막힌 게 없다면 원래 가려던 곳으로
		TargetLocation = DesiredTargetLocation;
	}

	FVector CurrentLocation = DroneMesh->GetComponentLocation();
	float DistanceToPlayer = FVector::Dist(PlayerLocation, CurrentLocation);

	if (DistanceToPlayer > 1000.0f)
	{
		// 너무 멀어지면 텔레포트
		DroneMesh->SetWorldLocation(TargetLocation);
		CurrentLocation = TargetLocation;
	}
	else
	{
		// 벽을 뚫지 않는 안전한 TargetLocation으로 부드럽게 이동
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, FollowSpeed);
		DroneMesh->SetWorldLocation(NewLocation);
		CurrentLocation = NewLocation;
	}

	FRotator BaseTargetRotation = YawOnlyRotation;

	if (CurrentTarget)
	{
		// 무조건 타겟을 바라봄
		FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - CurrentLocation).GetSafeNormal();
		BaseTargetRotation = DirectionToTarget.Rotation();
	}
	else
	{
		// 플레이어 앞을 보면서 비행기처럼 관성 기울기 + 숨쉬기 적용
		// 드론이 목표를 향해 얼마나 가야 하는지(이동 벡터)
		FVector MoveDelta = TargetLocation - CurrentLocation;
		float Distance = MoveDelta.Size();

		if (Distance > 30.0f) {
			BaseTargetRotation = MoveDelta.Rotation();

			float TargetPitch = FMath::Clamp(Distance * -0.1f, -45.0f, 0.0f);
			BaseTargetRotation.Pitch += TargetPitch;
		}
		else {
			BaseTargetRotation = YawOnlyRotation;

			if (!bIsAttacking)
			{
				BaseTargetRotation.Pitch += FMath::Sin(GetWorld()->GetTimeSeconds() * 1.5f) * 2.0f;
				BaseTargetRotation.Roll += FMath::Cos(GetWorld()->GetTimeSeconds() * 1.2f) * 2.0f;
			}
		}
	}

	FRotator CurrentRotation = DroneMesh->GetComponentRotation();

	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, BaseTargetRotation, DeltaTime, FollowSpeed * 1.5f);
	DroneMesh->SetWorldRotation(NewRotation);
}

void UDroneComponent::OnAttackTick()
{
	// 레이더에 잡힌 적이 없다면 즉시 종료
	if (!CurrentTarget) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < CurrentStats.AttackSpeed)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Drone Attack"));

	if (CurrentTarget)
	{
		FVector DroneForward = DroneMesh->GetForwardVector(); // 드론이 현재 바라보는 정면
		FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - DroneMesh->GetComponentLocation()).GetSafeNormal();

		float Alignment = FVector::DotProduct(DroneForward, DirectionToTarget);

		if (Alignment >= 0.95f)
		{
			bIsAttacking = true;
			LastFireTime = CurrentTime;

			UGameplayStatics::ApplyDamage(
				CurrentTarget,
				CurrentStats.AttackDamage,
				GetOwner()->GetInstigatorController(),
				GetOwner(),                           
				UDamageType::StaticClass()
			);

			if (CurrentStats.MuzzleFlashVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					CurrentStats.MuzzleFlashVFX,
					DroneMesh,
					TEXT("MuzzleSocket"),
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true
				);
			}


			// 공격이 끝나면 다시 호버링
			FTimerHandle AttackResetTimer;
			GetWorld()->GetTimerManager().SetTimer(AttackResetTimer, FTimerDelegate::CreateLambda([this]()
				{
					bIsAttacking = false;
				}), 0.2f, false);
		}
	}
}

void UDroneComponent::OnHealTick()
{
	UE_LOG(LogTemp, Log, TEXT("Drone Heal Owner"));

	if (UAttributeComponent* AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>()) {
		AttributeComp->Heal(CurrentStats.HealAmount);
	}
}

void UDroneComponent::ActiveDroneSkill()
{
	if (!bCanUseSkill) {
		UE_LOG(LogTemp, Warning, TEXT("Drone Skill CoolTime"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Drone Skill Active"));

	bool bSkillExecuted = false;

	switch (CurrentStats.CurrentActiveSkill) {
	case EDroneActiveSkill::Knockback:
		DoKnockback();
		bSkillExecuted = true;
		break;
		
	case EDroneActiveSkill::BerserkerBuff:
		/* 추후 구현*/
		break;

	default:
		break;
	}


	if (bSkillExecuted) {
		// 스킬 사용 후 쿨타임동안 사용 불가
		bCanUseSkill = false;

		// 쿨타임이 지난 후 스킬 사용 가능하게 수정
		GetWorld()->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UDroneComponent::OnCooldownFinished,
			CurrentStats.ActiveSkillCooldownTime,
			false
		);
	}
}

bool UDroneComponent::TryActivateRevive()
{
	if (!bCanUseRevive || CurrentDroneState == EDroneState::Reviving) return false;

	bCanUseRevive = false;
	CurrentDroneState = EDroneState::Reviving;

	// 주변을 밀쳐냄
	DoKnockback();
	UE_LOG(LogTemp, Warning, TEXT("Warning, Player Down! Revive Protocol operate"));

	FTimerHandle ReviveProcessHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ReviveProcessHandle,
		this,
		&UDroneComponent::OnReviveProcessFinished,
		CurrentStats.ReviveDuration,
		false
	);

	return true;
}

void UDroneComponent::DoKnockback()
{
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	float Radius = CurrentStats.KnockbackRange;
	float Force = CurrentStats.KnockbackForce;

	// 디버그
	DrawDebugSphere(GetWorld(), PlayerLocation, CurrentStats.KnockbackRange, 32, FColor::Red, false, 2.0f);

	// 충돌 검사
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Enemy));

	bool bResult = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ObjectTypes,
		Sphere
	);

	if (!bResult) return;

	for (auto& Result : OverlapResults) {
		AActor* HitActor = Result.GetActor();

		if (HitActor && HitActor != GetOwner()) {
			ACharacter* EnemyChar = Cast<ACharacter>(HitActor);

			if (EnemyChar) {
				// 밀어낼 방향 (Player -> Enemy 방향)
				FVector Direction = (EnemyChar->GetActorLocation() - PlayerLocation).GetSafeNormal();

				// 살짝 띄워서
				Direction.Z = 0.5f;
				FVector LaunchVelocity = Direction * Force;

				EnemyChar->GetCharacterMovement()->StopMovementImmediately();

				// 날리기
				EnemyChar->LaunchCharacter(LaunchVelocity, true, true);
			}
		}

	}
}

void UDroneComponent::OnCooldownFinished()
{
	bCanUseSkill = true;
}

void UDroneComponent::OnReviveProcessFinished()
{
	CurrentDroneState = EDroneState::Idle;
	UE_LOG(LogTemp, Log, TEXT("Revive Complete"));

	// 부활 완료 방송
	if (OnReviveComplete.IsBound())
	{
		OnReviveComplete.Broadcast(CurrentStats.ReviveHealthPercent);
	}

	GetWorld()->GetTimerManager().SetTimer(
		ReviveCooldownTimerHandle,
		this,
		&UDroneComponent::OnReviveCooldownFinished,
		CurrentStats.ReviveCooldown,
		false
	);
}

void UDroneComponent::OnReviveCooldownFinished()
{
	bCanUseRevive = true;
}

void UDroneComponent::PerformRadarScan()
{
	// 드론 현재 위치와 스캔 반경
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	float RadarRadius = CurrentStats.AttackRange;

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Enemy);

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(RadarRadius);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape
	);

	// 임시 명단
	TArray<AActor*> NewlyDetectedEnemies;

	if (bHit) {
		for (const FOverlapResult& Result : OverlapResults) {
			AActor* HitActor = Result.GetActor();
			if (HitActor && HitActor != GetOwner()) {
				NewlyDetectedEnemies.AddUnique(HitActor);
			}
		}
	}

	for (AActor* OldEnemy : EnemiesInRange) {
		if (!NewlyDetectedEnemies.Contains(OldEnemy)) {
			OnRadarEndOverlap(nullptr, OldEnemy, nullptr, 0);
		}
	}

	for (AActor* NewEnemy : NewlyDetectedEnemies) {
		if (!EnemiesInRange.Contains(NewEnemy)) {
			OnRadarBeginOverlap(nullptr, NewEnemy, nullptr, 0, false, FHitResult());
		}
	}

	// 과거 명단을 현재로 교체
	EnemiesInRange = NewlyDetectedEnemies;
}

void UDroneComponent::OnRadarBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 적이 들어오면 배열에 추가 (AddUnique로 중복 방지)
	if (OtherActor && OtherActor != GetOwner())
	{
		EnemiesInRange.AddUnique(OtherActor);
	}
}

void UDroneComponent::OnRadarEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 적이 나가면 배열에서 제거
	if (OtherActor)
	{
		EnemiesInRange.Remove(OtherActor);
	}
}

void UDroneComponent::FindBestTarget()
{
	if (EnemiesInRange.IsEmpty())
	{
		CurrentTarget = nullptr;
		return;
	}

	AActor* BestTarget = nullptr;
	float MinDistSq = MAX_flt;
	FVector DroneLocation = DroneMesh->GetComponentLocation();
	FVector PlayerLocation = GetOwner()->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	// 배열 역순 순회
	for (int32 i = EnemiesInRange.Num() - 1; i >= 0; i--)
	{
		AActor* Target = EnemiesInRange[i];
		if (!IsValid(Target))
		{
			EnemiesInRange.RemoveAt(i);
			continue;
		}

		float DistSq = FVector::DistSquared(PlayerLocation, Target->GetActorLocation());

		if (DistSq < MinDistSq)
		{
			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				DroneLocation,
				Target->GetActorLocation(),
				ECC_PlayerProjectile,
				QueryParams
			);

			if (bHit && HitResult.GetActor() == Target)
			{
				MinDistSq = DistSq; 
				BestTarget = Target;
			}
		}
	}

	CurrentTarget = BestTarget;
}

void UDroneComponent::UpdateDroneTimers()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (CurrentDroneState == EDroneState::Reviving) return;

	World->GetTimerManager().SetTimer(TargetSearchTimer, this, &UDroneComponent::FindBestTarget, 0.2f, true);

	if (CurrentStats.bHasAutoAttack) {
		// 자동 공격 기능이 켜졌는지 확인 후 재설정
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
		World->GetTimerManager().SetTimer(AttackTimerHandle, this, &UDroneComponent::OnAttackTick, 0.1f, true);
	}
	else {
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}

	if (CurrentStats.bHasAutoHeal) {
		// 자동 공격 기능이 켜졌는지 확인 후 재설정
		World->GetTimerManager().ClearTimer(HealTimerHandle);
		World->GetTimerManager().SetTimer(HealTimerHandle, this, &UDroneComponent::OnHealTick, CurrentStats.HealInterval, true);
	}
	else {
		World->GetTimerManager().ClearTimer(HealTimerHandle);
	}
}

void UDroneComponent::ApplyUpgrade(EDroneUpgradeType Type, float Value)
{
	// 각 타입에 맞는 드론 스탯 증가
	switch (Type) {
	case EDroneUpgradeType::Range:
		CurrentStats.KnockbackRange += Value;
		break;

	case EDroneUpgradeType::Force:
		CurrentStats.KnockbackForce += Value;
		break;

	case EDroneUpgradeType::AttackDamage:
		CurrentStats.AttackDamage += Value;
		break;

	case EDroneUpgradeType::HealAmount:
		CurrentStats.HealAmount += Value;
		break;

	case EDroneUpgradeType::Cooldown:
		CurrentStats.ActiveSkillCooldownTime += Value;
		CurrentStats.ActiveSkillCooldownTime = FMath::Max(0.1f, CurrentStats.ActiveSkillCooldownTime);
		break;

	case EDroneUpgradeType::AttackSpeed:
		CurrentStats.AttackSpeed += Value;
		CurrentStats.AttackSpeed = FMath::Max(0.1f, CurrentStats.AttackSpeed);

		// 타이머 갱신
		UpdateDroneTimers();
		break;

	case EDroneUpgradeType::HealInterval:
		CurrentStats.HealInterval -= Value;
		CurrentStats.HealInterval = FMath::Max(0.1f, CurrentStats.HealInterval);

		//  타이머 갱신
		UpdateDroneTimers();
		break;

	case EDroneUpgradeType::ReviveAmount:
		CurrentStats.ReviveHealthPercent += Value;
		CurrentStats.ReviveHealthPercent = FMath::Min(100.0f, CurrentStats.ReviveHealthPercent);
		break;


	case EDroneUpgradeType::UnlockAttack:
		// Value가 0이 아니면(1.0f 등) True로 취급
		CurrentStats.bHasAutoAttack = (Value > 0.0f);
		UpdateDroneTimers();
		break;
	
	case EDroneUpgradeType::UnlockHeal:
		CurrentStats.bHasAutoHeal = (Value > 0.0f);
		UpdateDroneTimers();
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("[Upgrade] Type: %d, Value applied: %f"), (int32)Type, Value);
}

void UDroneComponent::SaveDataToGI(USurvivalGameInstance* GI)
{
	if (!GI) return;

	GI->SavedDroneStats = CurrentStats;
}

void UDroneComponent::LoadDataFromGI(USurvivalGameInstance* GI)
{
	if (!GI || !GI->bIsSaveDataValid) return;

	CurrentStats = GI->SavedDroneStats;
}