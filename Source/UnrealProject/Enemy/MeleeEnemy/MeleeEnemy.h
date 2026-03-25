// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BaseEnemy.h"
#include "../DataTable/MeleeEnemyStatRow.h"
#include "MeleeEnemy.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AMeleeEnemy : public ABaseEnemy
{
	GENERATED_BODY()
	
public:
	AMeleeEnemy();

public:
	virtual void OnAttack() override;

public:
	virtual void InitEnemyData() override;

	// 게임 시작 시 엑셀에서 뽑아온 원본 데이터를 복사해서 들게 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee|Stat")
	FMeleeEnemyStatRow CurrentMeleeStat;

	// 여러 개의 근접 공격 모션을 배열로 담아놓는다.
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<UAnimMontage*> MeleeAttackMontages;*/
};
