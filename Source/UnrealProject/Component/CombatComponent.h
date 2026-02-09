// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Weapon/BaseWeapon.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponChangedDelegate, class ABaseWeapon*, NewWeapon);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 무기를 안전한 위치에 떨어뜨리는 내부 함수
	void DropWeaponSafeLocation(class ABaseWeapon* WeaponToDrop);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 무기 획득
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PickupWeapon(ABaseWeapon* NewWeapon);
	
	// 무기 장착
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeaponBySlot(EWeaponSlot SlotToEquip);

	void Attack();

	void StopAttack();

	void Reload();

public:
	ABaseWeapon* GetCurrentWeapon() {
		return CurrentWeapon;
	}

protected:
	// 무기를 각 슬롯에 맞게 저장할 인벤토리
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Inventory")
	TMap<EWeaponSlot, ABaseWeapon*> CarriedWeapons;

	// 현재 들고 있는 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	ABaseWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<class ABaseWeapon> DefaultWeaponClass;

	void AttachWeaponToHand(ABaseWeapon* Weapon);

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrentWeaponChangedDelegate OnCurrentWeaponChanged;
};
