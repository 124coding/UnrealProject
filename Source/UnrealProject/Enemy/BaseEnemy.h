// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Component/AttributeComponent.h"
#include "../HitInterface.h"
#include "../PoolableInterface.h"
#include "BaseEnemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	EES_Normal		UMETA(DisplayName = "Normal"),			// 평상시
	EES_Attacking	UMETA(DisplayName = "Attacking"),		// 공격 중
	EES_Stunned		UMETA(DisplayName = "Stunned"),			// 기절 
	EES_Dead		UMETA(DisplayName = "Dead")				// 사망
};

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

	// 근접공격
	virtual void Attack();

	// 공격 끝났는지 확인
	bool IsAttacking() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageAmount);

protected:
	// 공격 몽타주 끝났을때 호출
	UFUNCTION()
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 방향 판별 함수 선언
	void PlayDirectionalHitReact(const FVector& ImpactPoint);

public:
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual void OnPoolSpawned_Implementation() override;
	virtual void OnPoolReturned_Implementation() override;

	// 죽었을 때 실행할 함수
	UFUNCTION()
	void HandleDeath(AActor* VictimActor, AActor* KillerActor);

	// 풀로 돌아가는 함수
	void Deactivate();

protected:
	// 앞에서 맞았을 때
	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	UAnimMontage* HitReactMontage_Front;

	// 뒤에서 맞았을 때
	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	UAnimMontage* HitReactMontage_Back;

	// 왼쪽 맞음
	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	UAnimMontage* HitReactMontage_Left;

	// 오른쪽 맞음
	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	UAnimMontage* HitReactMontage_Right;

public:
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

};
