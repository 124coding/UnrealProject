// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_Footstep.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

UAN_Footstep::UAN_Footstep()
{
}

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = MeshComp->GetWorld();

	if (!World) return;

	FVector Location = OwnerActor->GetActorLocation();

	if (SoundToPlay) {
		UGameplayStatics::PlaySoundAtLocation(
			World, 
			SoundToPlay, 
			Location,
			VolumeMultiplier,
			PitchMultiplier);
	}

	UAISense_Hearing::ReportNoiseEvent(
		World,
		Location,
		NoiseLoudness * VolumeMultiplier,
		OwnerActor,
		NoiseMaxRange,
		NoiseTag
	);

	/* 바닥 재질에 따라 소리 분리 추가 가능 */
}
