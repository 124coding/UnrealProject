// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Weapon/BaseWeapon.h"
#include "../EnumTypes/WeaponTypes.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponChangedDelegate, class ABaseWeapon*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReserveAmmoChanged, int32, NewReserveAmmo);

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

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ammo")
	int32 MaxARAmmo = 200;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ammo")
	int32 MaxLCAmmo = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ammo")
	int32 MaxSGAmmo = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ammo")
	int32 MaxSRAmmo = 50;

	UFUNCTION(BlueprintCallable, Category = "Combat|Ammo")
	void CarriedAmmoFill();

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

	void CycleWeapon(bool bScrollUp);

	// 무기 장전 완료 시 콜백 함수
	UFUNCTION()
	void HandleWeaponReloadFinished(int32 AmmoConsumed);

public:
	int32 CarriedWeaponsCount() {
		return CarriedWeapons.Num();
	}

	ABaseWeapon* GetCurrentWeapon() {
		return CurrentWeapon;
	}

	void DiscardEmptyWeapon();

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

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnReserveAmmoChanged OnReserveAmmoChanged;

	// 플레이어가 현재 들고 있는 여분 탄약들
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Ammo")
	TMap<EAmmoType, int32> CarriedAmmo;

public:
	// GameInstance의 포인터를 받아서 알아서 자기 데이터를 넣고 빼는 함수
	void SaveDataToGI(class USurvivalGameInstance* GI);
	void LoadDataFromGI(USurvivalGameInstance* GI);
};
