// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractableBaseActor.h"
#include "../EnumTypes/WeaponTypes.h"
#include "DataTable/BaseWeaponStatRow.h"

#include "BaseWeapon.generated.h"

class UStaticMeshComponent;
class UInteractionComponent;
class USphereComponent;

UCLASS()
class UNREALPROJECT_API ABaseWeapon : public AInteractableBaseActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	// Set Weapon Type in Blueprint
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stat")
	EWeaponSlot WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;*/

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

	UInteractionComponent* GetInteractionComponent() {
		return InteractionComponent;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* WeaponMappingContext;

protected:

	virtual void InitWeaponData();

	// 에디터에서 엑셀 파일의 몇 번째 줄을 읽을지 선택하는 Handle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	FDataTableRowHandle WeaponDataHandle;

	// 게임 시작 시 엑셀에서 뽑아온 원본 데이터를 복사해서 들게 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stat")
	FBaseWeaponStatRow CurrentWeaponStat;

	// 마지막 공격시간
	double LastAttackTime = 0.0f;

	UPROPERTY()
	EWeaponState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* AreaSphere; // 상호작용 감지용(LineTrace 안쓸 때 대비)

	// 타이머
	FTimerHandle FireTimerHandle;

	// 점사 시 현재 발사 카운트
	int32 CurrentBurstCount = 0;

public:
	float GetKnockbackPower() const { return CurrentWeaponStat.KnockbackPower; }

	EWeaponSlot GetWeaponType() const {
		return CurrentWeaponStat.WeaponType;
	}
};
