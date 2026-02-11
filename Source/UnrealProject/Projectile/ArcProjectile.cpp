// Fill out your copyright notice in the Description page of Project Settings.


#include "ArcProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


void AArcProjectile::LaunchTowards(FVector StartLoc, AActor* TargetActor)
{
	if (!TargetActor || !ProjectileMovement) return;

	FVector TargetLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);
	FVector OutLaunchVelocity;

	// 곡사포 계산
	bool bHasAimSolution = UGameplayStatics::SuggestProjectileVelocity(
		this,
		OutLaunchVelocity,
		StartLoc,
		TargetLoc,
		ProjectileMovement->InitialSpeed,
		false,
		0.f,
		0.f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (bHasAimSolution) {
		UE_LOG(LogTemp, Log, TEXT("Aim Success"));
		ProjectileMovement->Velocity = OutLaunchVelocity;
		SetActorRotation(OutLaunchVelocity.Rotation());
	}
	else {
		// 계산 실패 시 부모의 직사로 대체
		Super::LaunchTowards(StartLoc, TargetActor);
		UE_LOG(LogTemp, Log, TEXT("Aim Fail"));
	}
}
