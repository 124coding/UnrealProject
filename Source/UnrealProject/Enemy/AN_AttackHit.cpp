// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_AttackHit.h"
#include "BaseEnemy.h"

UAN_AttackHit::UAN_AttackHit()
{
	// 기본 설정
	SocketName = TEXT("AttackSocket_hand_r");
	HalfRadiusSize = 40.0f;
	DamageAmount = 10.0f;
}

void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner()) {
		// 이 몽타주의 주인이 BaseEnemy인지 확인
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(MeshComp->GetOwner());

		if (Enemy) {
			Enemy->PerformMeleeAttackHitCheck(SocketName, HalfRadiusSize, DamageAmount);
		}
	}
}
