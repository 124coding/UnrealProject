// Fill out your copyright notice in the Description page of Project Settings.


#include "FootstepComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UFootstepComponent::UFootstepComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFootstep(DeltaTime);
}

void UFootstepComponent::UpdateFootstep(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 속도 가져오기
	float Speed = Owner->GetVelocity().Size2D();

	// 멈춰 있으면 초기화
	if (Speed < 10.f) {
		DistanceAccumulated = 0.f;
		return;
	}

	DistanceAccumulated += Speed * DeltaTime;

	if (DistanceAccumulated >= WalkStepDistance) {
		PlayFootstepSound();
		DistanceAccumulated = 0.f;
	}
}

void UFootstepComponent::PlayFootstepSound()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 속도에 따라 소리를 다르게
	float CurrentSpeed = Owner->GetVelocity().Size2D();
	FVector2D SpeedRange(MinStepSound, MaxStepSound);
	FVector2D VolumeRange(0.3f, 1.0f);
	float VolumeMultiplier = FMath::GetMappedRangeValueClamped(SpeedRange, VolumeRange, CurrentSpeed);

	// 피치음도 살짝 올라가게
	FVector2D PitchRange(0.9f, 1.1f);
	float PitchMultiplier = FMath::GetMappedRangeValueClamped(SpeedRange, PitchRange, CurrentSpeed);

	FHitResult HitResult;
	FVector Start = Owner->GetActorLocation();
	FVector End = Start - FVector(0, 0, ActorZOffset);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.bReturnPhysicalMaterial = true;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) {
		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

		if (FootstepSoundMap.Contains(SurfaceType)) {
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FootstepSoundMap[SurfaceType], 
				HitResult.ImpactPoint, 
				VolumeMultiplier,
				PitchMultiplier);

			Owner->MakeNoise(VolumeMultiplier, Cast<APawn>(Owner), HitResult.ImpactPoint);
		}
	}
}

void UFootstepComponent::PlayLandedSound(const FHitResult& HitResult, float FallSpeed)
{
	// 낙하 속도를 기반으로 볼륨 계산
	float VolumeMultiplier = FMath::Clamp(FallSpeed / 1000.0f, 0.2f, 1.0f);

	// 물리 재질 가져오기
	EPhysicalSurface SurfaceType = SurfaceType_Default;

	if (HitResult.PhysMaterial.IsValid()) {
		// 엔진이 넘겨준 정보에 재질이 있다면 바로 사용
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
	}
	else {
		// 정보가 비었다면 직접 레이캐스트
		AActor* Owner = GetOwner();
		if (Owner) {
			FHitResult HitResult;
			FVector Start = Owner->GetActorLocation();
			FVector End = Start - FVector(0.f, 0.f, ActorZOffset);

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Owner);
			Params.bReturnPhysicalMaterial = true;

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) {
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
			}
		}
	}

	USoundBase* SelectedSound = DefaultLandedSound;

	if (LandedSoundMap.Contains(SurfaceType))
	{
		SelectedSound = LandedSoundMap[SurfaceType];
	}

	if (SelectedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, HitResult.ImpactPoint, VolumeMultiplier);
	}
}

