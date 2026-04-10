// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Component/AttributeComponent.h"
#include "../Interface/HitInterface.h"
#include "../Interface/PoolableInterface.h"
#include "../EnumTypes/CharacterTypes.h"
#include "DataTable/BaseEnemyStatRow.h"
#include "BaseEnemy.generated.h"

UCLASS()
class UNREALPROJECT_API ABaseEnemy : public ACharacter, public IHitInterface, public IPoolableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 공격이 가능한지 확인
	virtual bool CanAttack();

	virtual void OnAttack();

	// 공격
	void Attack();

	// 공격 끝났는지 확인
	bool IsAttacking() const;

	// 강제로 타겟을 지정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetCommandTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageMultiplier);

protected:
	// 공격 몽타주 끝났을때 호출
	UFUNCTION()
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void Landed(const FHitResult& Hit) override;

	// 방향 판별 함수 선언
	void PlayDirectionalHitReact(const FVector& ImpactPoint);

public:
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPoolSpawned_Implementation() override;
	virtual void OnPoolReturned_Implementation() override;
	virtual void SetOwningPool_Implementation(UObjectPoolComponent* NewPool) override;

	// 죽었을 때 실행할 함수
	UFUNCTION()
	void HandleDeath(AActor* VictimActor, AActor* KillerActor);

	// 풀로 돌아가는 함수
	void Deactivate();

protected:
	//// 앞에서 맞았을 때
	//UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	//UAnimMontage* HitReactMontage_Front;

	//// 뒤에서 맞았을 때
	//UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	//UAnimMontage* HitReactMontage_Back;

	//// 왼쪽 맞음
	//UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	//UAnimMontage* HitReactMontage_Left;

	//// 오른쪽 맞음
	//UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	//UAnimMontage* HitReactMontage_Right;

	//UPROPERTY(EditAnywhere, Category = "Combat|Death")
	//UAnimMontage* DeathMontage;

	// 애니메이션을 굳히기 위한 타이머 핸들 및 함수
	FTimerHandle AnimFreezeTimerHandle;
	void FreezeAnimation();

protected:

	virtual void InitEnemyData();

	// 에디터에서 엑셀 파일의 몇 번째 줄을 읽을지 선택하는 Handle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data")
	FDataTableRowHandle EnemyDataHandle;

	// 게임 시작 시 엑셀에서 뽑아온 원본 데이터를 복사해서 들게 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stat")
	FBaseEnemyStatRow CurrentEnemyStat;

	// 공격 사거리
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
	//float AttackRange = 110.0f; // 근접 기준

public:
	UFUNCTION(BlueprintCallable)
	FBaseEnemyStatRow GetEnemyStat() {
		return CurrentEnemyStat;
	}

public:
	// 태어날 때의 메쉬 상태를 저장할 변수
	UPROPERTY()
	FTransform InitialMeshTransform;

	//// 해당 적의 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EEnemyType EnemyType = EEnemyType::Melee;

	// 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|State")
	EEnemyState CurrentState = EEnemyState::EES_Normal;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAttributeComponent* AttributeComponent;

	// 풀링 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UObjectPoolComponent* OwningPoolComponent;

	// 타이머핸들 (SetLifeSpan 대신 사용)
	FTimerHandle ReturnTimerHandle;

protected:
	// 주변에 겹쳐있는 다른 적들
	UPROPERTY()
	TArray<ABaseEnemy*> CachedNeighbors;

	// 계산된 밀어내기 힘을 저장할 변수
	FVector CurrentRepulsionForce;

	// 충돌 이벤트 바인딩용 함수
	UFUNCTION()
	void OnEnemyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnemyOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 가끔씩 실행할 계산 함수
	void CalculateSeparation();

protected:
	bool bIsActive = false;

public:
	virtual bool IsActiveInPool() const override
	{
		return bIsActive;
	}

	virtual void SetActiveInPool(bool bActive) override
	{
		bIsActive = bActive;
	}
};
