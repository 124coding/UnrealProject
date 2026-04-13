// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFootstepComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	TMap<TEnumAsByte<EPhysicalSurface>, class USoundBase*> FootstepSoundMap;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	class USoundBase* DefaultFootstepSound;


	// ÂøÁö »ç¿îµå
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landed")
	TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> LandedSoundMap;

	UPROPERTY(EditDefaultsOnly, Category = "Landed")
	class USoundBase* DefaultLandedSound;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	float MinStepSound = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	float MaxStepSound = 850.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	float WalkStepDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	float ActorZOffset = 150.f;

	void PlayLandedSound(const FHitResult& HitResult, float FallSpeed);

private:
	float DistanceAccumulated = 0.f;

	void UpdateFootstep(float DeltaTime);
	void PlayFootstepSound();
};
