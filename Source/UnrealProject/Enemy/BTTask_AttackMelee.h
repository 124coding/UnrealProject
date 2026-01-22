// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackMelee.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UBTTask_AttackMelee : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AttackMelee();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
