// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindRandomLocation.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
	NodeName = TEXT("Find Random Location");
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController->GetPawn();

	if (IsValid(AIPawn)) {
		// 적의 현재 위치
		FVector Origin = AIPawn->GetActorLocation();

		// 내비게이션 시스템
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

		if (IsValid(NavSystem)) {
			FNavLocation RandomRocation;

			// Origin 기준으로 반지름 안에서 랜덤한 점 찍기
			if (NavSystem->GetRandomPointInNavigableRadius(Origin, SearchRadius, RandomRocation)) {
				// 찾은 위치를 블랙보드 키로 저장
				OwnerComp.GetBlackboardComponent()->SetValueAsVector("PatrolLocation", RandomRocation.Location);

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
