// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../UnrealProject.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UDroneComponent::UDroneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDroneComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1초마다 패시브 로직 실행
	UpdateDroneTimers();
	// ...
	
}


// Called every frame
void UDroneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDroneComponent::OnAttackTick()
{
	UE_LOG(LogTemp, Log, TEXT("Drone Attack"));
	/* 구현 필요 */
}

void UDroneComponent::OnHealTick()
{
	UE_LOG(LogTemp, Log, TEXT("Drone Heal Owner"));
	/* 구현 필요 */
}

void UDroneComponent::ActiveDroneSkill()
{
	if (!bCanUseSkill) {
		UE_LOG(LogTemp, Warning, TEXT("Drone Skill CoolTime"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Drone Skill Active"));

	bool bSkillExecuted = false;

	switch (CurrentActiveSkill) {
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

	// Pawn 채널만 검사 (Enemy 전용 채널로 수정 필요)
	bool bResult = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ECC_Enemy,
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

void UDroneComponent::UpdateDroneTimers()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (CurrentDroneState == EDroneState::Reviving) return;

	if (CurrentStats.bHasAutoAttack) {
		// 자동 공격 기능이 켜졌는지 확인 후 재설정
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
		World->GetTimerManager().SetTimer(AttackTimerHandle, this, &UDroneComponent::OnAttackTick, 1 / CurrentStats.AttackSpeed, true);
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
		CurrentStats.ReviveHealthPercent = FMath::Max(100.0f, CurrentStats.ReviveHealthPercent);
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
