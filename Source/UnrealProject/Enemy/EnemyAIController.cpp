// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Navigation/CrowdFollowingComponent.h"

AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	// 군중 회피 관련 설정 가능
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BehaviorTreeAsset) {
		// 블랙보드 초기화
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

//void AEnemyAIController::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//    // 테스트용 플레이어 따라오기.
//	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
//    if (PlayerPawn)
//    {
//        // 2m 앞에서 멈춤
//        MoveToActor(PlayerPawn, 200.0f);
//    }
//}
