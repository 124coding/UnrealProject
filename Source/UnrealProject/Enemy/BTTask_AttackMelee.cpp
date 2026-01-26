// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackMelee.h"
#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"


UBTTask_AttackMelee::UBTTask_AttackMelee()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_AttackMelee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AI 컨트롤러 가져오기
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC == nullptr) return EBTNodeResult::Failed;

	// 컨트롤러가 조종하는 폰(적 캐릭터) 가져오기
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AIC->GetPawn());
	if (Enemy == nullptr) return EBTNodeResult::Failed;

	// 적에게 공격 명령 내리기
	Enemy->MeleeAttack();
	bIsAttacking = true;

	// 끝나지 않았다고 리턴
	return EBTNodeResult::InProgress;
}

void UBTTask_AttackMelee::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// AI 컨트롤러 가져오기
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC == nullptr) return;

	// 컨트롤러가 조종하는 폰(적 캐릭터) 가져오기
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AIC->GetPawn());
	if (Enemy == nullptr) return;

	if (Enemy && !Enemy->IsAttacking()) {
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
