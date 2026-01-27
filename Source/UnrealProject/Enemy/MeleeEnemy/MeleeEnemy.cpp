// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

void AMeleeEnemy::Attack()
{
	Super::Attack();

	if (MeleeAttackMontages.Num() > 0) {
		// 랜덤 숫자 뽑아서 당첨된 몽타주 가져와 재생
		int32 RandomIndex = FMath::RandRange(0, MeleeAttackMontages.Num() - 1);

		UAnimMontage* SelectedMontage = MeleeAttackMontages[RandomIndex];

		if (SelectedMontage) {
			PlayAnimMontage(SelectedMontage);
		}

	}
}
