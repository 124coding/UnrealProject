// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Projectile/ArcProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Component/ObjectPoolComponent.h"
#include "../../UnrealProjectGameMode.h"

ARangedEnemy::ARangedEnemy()
{
	EnemyType = EEnemyType::Ranged;
	AttackRange = 1000.0f;
}

void ARangedEnemy::Attack()
{
	if (CurrentState == EEnemyState::EES_Attacking || CurrentState == EEnemyState::EES_Dead)
	{
		return;
	}

	CurrentState = EEnemyState::EES_Attacking;

	Super::Attack();

	if (RangedAttackMontage) {
		PlayAnimMontage(RangedAttackMontage);
	}

}

void ARangedEnemy::FireProjectile()
{
	if (ProjectileClass) {
		// 발사 위치
		FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("AttackSocket_hand_r"));

		AAIController* AIC = Cast<AAIController>(GetController());
		
		// 플레이어 위치로 발사
		if (AIC && AIC->GetBlackboardComponent()) {
			AActor* TargetActor = Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
			
			FRotator SpawnRotation = GetActorForwardVector().Rotation();

			if (TargetActor) {
				FVector TargetLocation = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);
				SpawnRotation = (TargetLocation - SpawnLocation).Rotation();
			}

			// 풀에서 꺼낸 후 투사체가 알아서 경로 설정
			if (AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode())) {
				AActor* SpawnedActor = GM->SpawnProjectileFromPool(ProjectileClass, SpawnLocation, SpawnRotation);

				if (AArcProjectile* Proj = Cast<AArcProjectile>(SpawnedActor)) {
					if (TargetActor) {
						Proj->LaunchTowards(SpawnLocation, TargetActor);
					}
				}
			}
		}
	}
}

void ARangedEnemy::FireAtTarget()
{
}
