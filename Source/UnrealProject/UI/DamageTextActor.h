// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/PoolableInterface.h"
#include "DamageTextActor.generated.h"

UCLASS()
class UNREALPROJECT_API ADamageTextActor : public AActor, public IPoolableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamageTextActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* DamageWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void SetupDamageText(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDamage(const FVector& Location, float DamageAmount);

	virtual void OnPoolSpawned_Implementation() override;
	virtual void OnPoolReturned_Implementation() override;
	virtual void SetOwningPool_Implementation(UObjectPoolComponent* NewPool) override;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// 풀링 컴포넌트
	UPROPERTY()

	class UObjectPoolComponent* OwningPoolComponent;

	FTimerHandle ReturnTimerHandle;

	// 타이머가 끝나면 풀로 스스로 돌아가는 함수
	void SelfReturnToPool();

};
