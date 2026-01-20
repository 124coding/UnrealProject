// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponSlot : uint8 {
	Primary		UMETA(DisplayName = "Primary Weapon"),
	Secondary	UMETA(DisplayName = "Secondary Weapon"),
	Melee		UMETA(DisplayName = "Melee Weapon"),
	Throwable	UMETA(Display = "Throwable"),
	MAX			UMETA(Hidden)
};

UCLASS()
class UNREALPROJECT_API ABaseWeapon : public AActor
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
	virtual void Attack();

	UStaticMeshComponent* GetWeaponMesh() {
		return WeaponMesh;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* WeaponMappingContext;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float AttackRate = 0.5f;

};
