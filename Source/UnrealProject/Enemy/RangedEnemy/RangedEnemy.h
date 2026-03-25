// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BaseEnemy.h"
#include "../DataTable/RangedEnemyStatRow.h"
#include "RangedEnemy.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API ARangedEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	ARangedEnemy();

	virtual void OnAttack() override;

protected:

	virtual void InitEnemyData() override;

	// 게임 시작 시 엑셀에서 뽑아온 원본 데이터를 복사해서 들게 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ranged|Stat")
	FRangedEnemyStatRow CurrentRangedStat;

public:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	//TSubclassOf<class AUnrealProjectProjectile> ProjectileClass;

	//// 원거리 공격 몽타주
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	//UAnimMontage* RangedAttackMontage;
	
	// 애님 노티파이가 부를 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireProjectile();
};
