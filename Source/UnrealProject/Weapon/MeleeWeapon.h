// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "MeleeWeapon.generated.h"

/**
  미완성
 */
UCLASS()
class UNREALPROJECT_API AMeleeWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AMeleeWeapon();
public:
	// void TickAttack();

	virtual bool CanAttack() override;

	virtual void ExecuteFire() override;

	void EndAttack();

protected:
	// 애니메이션을 이용한 공격이 아니기 때문에 다음의 것들은 필요치 않음
	// 이미 때린 적을 기억하는 배열
	/*UPROPERTY()
	TArray<AActor*> IgnoreActors;*/
	
	// 이전 프레임의 소켓 위치 저장용
	/*FVector LastTipLocation;
	FVector LastBaseLocation;*/

	// 스윕 반지름 (칼 두께)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TraceRadius = 20.0f;
};
