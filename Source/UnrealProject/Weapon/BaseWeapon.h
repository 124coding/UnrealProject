// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interactable.h"
#include "BaseWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	Equipped	UMETA(DisplayName = "Equipped"),
	Dropped		UMETA(DisplayName = "Dropped")
};

UENUM(BlueprintType)
enum class EWeaponSlot : uint8 {
	Primary		UMETA(DisplayName = "Primary Weapon"),
	Secondary	UMETA(DisplayName = "Secondary Weapon"),
	Melee		UMETA(DisplayName = "Melee Weapon"),
	Throwable	UMETA(DisplayName = "Throwable"),
	MAX			UMETA(Hidden)
};

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

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	// 상태 변경
	void SetWeaponState(EWeaponState NewState);

	UStaticMeshComponent* GetWeaponMesh() {
		return WeaponMesh;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* WeaponMappingContext;

protected:
	UPROPERTY()
	EWeaponState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AreaSphere; // 상호작용 감지용(LineTrace 안쓸 때 대비)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float AttackRate = 0.5f;

	// 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float AttackRange = 5000.0f;

};
