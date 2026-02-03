// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

// 체력이 0이 되었을 때를 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathSignature, AActor*, Victim, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const {
		return MaxHealth;
	}

	UFUNCTION(BlueprintCallable)
	float GetHealth() const{
		return CurrentHealth;
	}

protected:

	// 최대 체력
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.0f;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, Category = "Health")
	float CurrentHealth;
		
public:
	// 데미지를 받는 함수 (OnTakeAny Damage 델리게이트와 모양이 똑같아야 연결 가능)
	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// 최대 체력 증가
	UFUNCTION(BlueprintCallable, Category = "Health")
	void MaxHealthUp(float Amount);

	// 회복 로직
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
};
