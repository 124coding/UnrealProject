// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/Interactable.h"
#include "../EnumTypes/WeaponTypes.h"
#include "BaseWeapon.generated.h"

UCLASS()
class UNREALPROJECT_API ABaseWeapon : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	// Set Weapon Type in Blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stat")
	EWeaponSlot WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void Attack();

	virtual bool CanAttack();

	virtual void OnAttack();

	virtual void StopAttack();

	virtual void ExecuteFire();

	// 점사 로직 처리 함수
	void HandleBurstFire();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	// 상태 변경
	void SetWeaponState(EWeaponState NewState);

	UStaticMeshComponent* GetWeaponMesh() {
		return WeaponMesh;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* WeaponMappingContext;

protected:
	// 마지막 공격시간
	double LastAttackTime = 0.0f;

	UPROPERTY()
	EWeaponState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AreaSphere; // 상호작용 감지용(LineTrace 안쓸 때 대비)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|WeaponStat")
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|WeaponStat")
	float AttackRate = 0.5f;

	// 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|WeaponStat")
	float AttackRange = 5000.0f;

	// 공격 모드
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	EWeaponFireMode FireMode = EWeaponFireMode::Single;

	// 점사 시 한번에 공격하는 횟수
	UPROPERTY(EditDefaultsOnly, Category = "Combat|FireMode", meta = (EditCondition = "FireMode == EWeaponFireMode::Burst"))
	int32 MaxBurstCount = 3;

	// 점사 시 공격 속도
	UPROPERTY(EditDefaultsOnly, Category = "Combat|FireMode", meta = (EditCondition = "FireMode == EWeaponFireMode::Burst"))
	float BurstFireRate = 0.2;

	// 타이머
	FTimerHandle FireTimerHandle;

	// 점사 시 현재 발사 카운트
	int32 CurrentBurstCount = 0;
};
