// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_AttackHit.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UAN_AttackHit : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAN_AttackHit();

	// 공격의 타격점이 될 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HalfRadiusSize;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
