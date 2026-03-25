// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../EnumTypes/DroneTypes.h"

#include "Components/SphereComponent.h"

#include "Engine/DataTable.h"

#include "DroneComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDroneReviveComplete, float, RevivePercent);

// 드론의 모든 스탯과 기능 잠금 여부를 관리하는 구조체 <-- 추후 능력 확장을 위함
USTRUCT(BlueprintType) 
struct FDroneStats : public FTableRowBase{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ActiveSkillCooldownTime = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float KnockbackRange = 500.0f; // 넉백 범위

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float KnockbackForce = 1000.0f; // 넉백 파워

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHasAutoAttack = false; // 자동 공격 가능 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHasAutoHeal = false; // 힐 가능 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AttackDamage = 10.0f; // 데미지

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HealAmount = 5.0f; // 힐량

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AttackSpeed = 0.5f; // 공격속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HealInterval = 5.0f; // 힐 주기

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ReviveDuration = 10.0f; // 부활 걸리는 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ReviveCooldown = 300.0f; // 부활 스킬 쿨타임

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "25", UIMin = "100", ForceUnits = "%"))
	float ReviveHealthPercent = 25.0f; // 부활 시 채워줄 체력 퍼센트

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drone|Skill")
	EDroneActiveSkill CurrentActiveSkill;

	// 에디터에서 드론의 외형을 갈아 끼울 수 있는 슬롯
	UPROPERTY(EditDefaultsOnly, Category = "Drone | Visual")
	class UStaticMesh* DroneMeshAsset;

	// 머즐 플래시(총구 화염) 이펙트 에셋을 받을 변수
	UPROPERTY(EditDefaultsOnly, Category = "Drone | VFX")
	class UNiagaraSystem* MuzzleFlashVFX;
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
	UPROPERTY(EditDefaultsOnly, Category = "Drone | Data")
	FDataTableRowHandle DroneDataHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drone")
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

protected:
	//// 드론의 레이더 역할을 할 구체 컴포넌트
	//UPROPERTY(VisibleAnywhere, Category = "Drone | Radar")
	//USphereComponent* DetectionSphere;

	FTimerHandle RadarScanTimerHandle;

	// 현재 레이더 반경 안에 있는 적들을 캐싱(기억)할 배열
	UPROPERTY()
	TArray<AActor*> EnemiesInRange;

	void PerformRadarScan();

	// 적이 들어오고 나갈 때 발동할 이벤트 함수
	UFUNCTION()
	void OnRadarBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRadarEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 드론의 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone | Visual")
	UStaticMeshComponent* DroneMesh;

protected:
	// 플레이어를 쫓아가는 속도 (낮을수록 더 늦게/무겁게 따라옴)
	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float FollowSpeed = 3.0f;

	// 플레이어 기준 드론의 목표 위치
	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	FVector HoverOffset = FVector(80.0f, 70.0f, 80.0f);

	// 현재 드론이 노리고 있는 가장 가까운 타겟
	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	// 현재 공격 중인지 확인하는 변수
	bool bIsAttacking = false;

	// 마지막 발사 시간
	float LastFireTime = 0.0f;

	// 0.2초마다 실행될 타겟 탐색 함수
	void FindBestTarget();

	// 타이머 핸들
	FTimerHandle TargetSearchTimer;

public:
	void UpdateDroneTimers();
	
	// 상점 등 드론 업그레이드 시 호출 될 함수
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ApplyUpgrade(EDroneUpgradeType Type, float Value);

public:

	UPROPERTY(BlueprintAssignable, Category = "Drone|Event")
	FOnDroneReviveComplete OnReviveComplete;

public:
	// GameInstance의 포인터를 받아서 알아서 자기 데이터를 넣고 빼는 함수
	void SaveDataToGI(class USurvivalGameInstance* GI);
	void LoadDataFromGI(USurvivalGameInstance* GI);
};
