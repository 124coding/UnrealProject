// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../../EnumTypes/CharacterTypes.h"
#include "BaseEnemyStatRow.h"
#include "MeleeEnemyStatRow.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FMeleeEnemyStatRow : public FBaseEnemyStatRow
{
	GENERATED_BODY()
public:

	// 여러 개의 근접 공격 모션을 배열로 담아놓는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	TArray<UAnimMontage*> MeleeAttackMontages;
};
