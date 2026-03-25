// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../../EnumTypes/CharacterTypes.h"
#include "BaseEnemyStatRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBaseEnemyStatRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	// 앞에서 맞았을 때
	UPROPERTY(EditDefaultsOnly, Category = "Base|Hit")
	UAnimMontage* HitReactMontage_Front;

	// 뒤에서 맞았을 때
	UPROPERTY(EditDefaultsOnly, Category = "Base|Hit")
	UAnimMontage* HitReactMontage_Back;

	// 왼쪽 맞음
	UPROPERTY(EditDefaultsOnly, Category = "Base|Hit")
	UAnimMontage* HitReactMontage_Left;

	// 오른쪽 맞음
	UPROPERTY(EditDefaultsOnly, Category = "Base|Hit")
	UAnimMontage* HitReactMontage_Right;

	UPROPERTY(EditDefaultsOnly, Category = "Base|Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Base")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Base")
	float AttackRange = 110.0f; // 근접 기준
};
