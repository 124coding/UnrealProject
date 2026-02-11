// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

AMeleeEnemy::AMeleeEnemy()
{
	EnemyType = EEnemyType::Melee;
}

void AMeleeEnemy::Attack()
{
	if (CurrentState == EEnemyState::EES_Attacking || CurrentState == EEnemyState::EES_Dead)
	{
		return;
	}

	CurrentState = EEnemyState::EES_Attacking;

	Super::Attack();

	if (MeleeAttackMontages.Num() > 0) {
		// 랜덤 숫자 뽑아서 당첨된 몽타주 가져와 재생
		int32 RandomIndex = FMath::RandRange(0, MeleeAttackMontages.Num() - 1);

		UAnimMontage* SelectedMontage = MeleeAttackMontages[RandomIndex];

		if (SelectedMontage) {
			PlayAnimMontage(SelectedMontage);

			//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			//if (AnimInstance) {
			//	// 기존에 예약이 있다면 제거(안전장치)
			//	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMeleeEnemy::OnMontageEnded);

			//	// 새 예약 걸기
			//	AnimInstance->OnMontageEnded.AddDynamic(this, &AMeleeEnemy::OnMontageEnded);
			//}
		}

	}
}
