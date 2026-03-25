// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../../EnumTypes/CharacterTypes.h"
#include "BaseEnemyStatRow.h"
#include "RangedEnemyStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FRangedEnemyStatRow : public FBaseEnemyStatRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	TSubclassOf<class AUnrealProjectProjectile> ProjectileClass;

	// 원거리 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ranged")
	UAnimMontage* RangedAttackMontage;
};
