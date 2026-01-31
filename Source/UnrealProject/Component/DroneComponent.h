// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDroneReviveComplete, float, RevivePercent);

UENUM(BlueprintType)
enum class EDroneState : uint8 {
	Idle,		// 평상 시
	Reviving	// 부활 작업 중
};

UENUM(BlueprintType)
enum class EDroneActiveSkill : uint8 {
	None		    UMETA(DisplayName = "None"),
	Knockback		UMETA(DisplayName = "Explosive Blast"),
	BerserkerBuff	UMETA(DisplayName = "BerserkerBuff") // 추후 구현
};

UENUM(BlueprintType)
enum class EDroneUpgradeType : uint8 {
	Range			UMETA(DisplayName = "Range"),
	Force			UMETA(DisplayName = "Knockback Force"),
	Cooldown		UMETA(DisplayName = "Cooldown Reduction"),
	UnlockAttack	UMETA(DisplayName = "Unlock Auto Attack"),
	UnlockHeal		UMETA(DisplayName = "Unlock Heal"),
	AttackDamage	UMETA(DisplayName = "Attack Damage"),
	AttackSpeed		UMETA(DisplayName = "Attack Speed"),
	HealAmount		UMETA(DisplayName = "Heal Amount"),
	HealInterval	UMETA(DisplayName = "HealInterval"),
	ReviveAmount	UMETA(DisplayName = "ReviveAmount")
};

// 드론의 모든 스탯과 기능 잠금 여부를 관리하는 구조체 <-- 추후 능력 확장을 위함
USTRUCT(BlueprintType) 
struct FDroneStats{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveSkillCooldownTime = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackRange = 500.0f; // 넉백 범위

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackForce = 1000.0f; // 넉백 파워

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAutoAttack = false; // 자동 공격 가능 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAutoHeal = false; // 힐 가능 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDamage = 10.0f; // 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealAmount = 5.0f; // 힐량

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 0.5f; // 공격속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealInterval = 5.0f; // 힐 주기

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReviveDuration = 10.0f; // 부활 걸리는 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReviveCooldown = 300.0f; // 부활 스킬 쿨타임

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "25", UIMin = "100", ForceUnits = "%"))
	float ReviveHealthPercent = 25.0f; // 부활 시 채워줄 체력 퍼센트
	/* 다양하게 확장 가능 */
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UDroneComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDroneComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FDroneStats CurrentStats;
	
	EDroneState CurrentDroneState = EDroneState::Idle; // 현재 상태

	FTimerHandle AttackTimerHandle;
	FTimerHandle HealTimerHandle;

	FTimerHandle CooldownTimerHandle; // 기본 액티브 쿨타임
	FTimerHandle ReviveCooldownTimerHandle; // 부활 쿨타임
	bool bCanUseSkill = true;
	bool bCanUseRevive = true;

public:
	void ActiveDroneSkill();
	bool TryActivateRevive();

protected:
	void OnAttackTick();
	void OnHealTick();

	void DoKnockback();

	// 드론 ActiveSkill 사용 가능하게 하는 함수
	void OnCooldownFinished();

	// 부활 완료 함수
	void OnReviveProcessFinished();

	// 부활 스킬 쿨다운 완료 함수
	void OnReviveCooldownFinished();

public:
	void UpdateDroneTimers();
	
	// 상점 등 드론 업그레이드 시 호출 될 함수
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ApplyUpgrade(EDroneUpgradeType Type, float Value);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Skill")
	EDroneActiveSkill CurrentActiveSkill = EDroneActiveSkill::Knockback;

	UPROPERTY(BlueprintAssignable, Category = "Drone|Event")
	FOnDroneReviveComplete OnReviveComplete;
};
