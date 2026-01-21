// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Component/AttributeComponent.h"
#include "BaseEnemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	EES_Idle		UMETA(DisplayName = "Idle"),			// 평상시
	EES_Patrolling  UMETA(DisplayName = "Patrolling"),		// 순찰 중
	EES_Chasing		UMETA(DisplayName = "Chasing"),			// 추격 중
	EES_Attacking	UMETA(DisplayName = "Attacking"),		// 공격 중
	EES_HitReact    UMETA(DisplayName = "HitReact"),		// 피격
	EES_Stunned		UMETA(DisplayName = "Stunned"),			// 기절 
	EES_Dead		UMETA(DisplayName = "Dead")				// 사망
};

UCLASS()
class UNREALPROJECT_API ABaseEnemy : public ACharacter
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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 죽었을 때 실행할 함수
	UFUNCTION()
	void HandleDeath();

public:
	// 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|State")
	EEnemyState CurrentState = EEnemyState::EES_Idle;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAttributeComponent* AttributeComponent;

};
