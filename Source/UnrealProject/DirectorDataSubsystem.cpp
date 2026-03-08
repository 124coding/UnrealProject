// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectorDataSubsystem.h"

void UDirectorDataSubsystem::AddStressEvent(float StressAmount)
{
	CurrentPlayerStress += StressAmount;
	CurrentPlayerStress = FMath::Clamp(CurrentPlayerStress, 0, 100);
}

void UDirectorDataSubsystem::DecayStress(float DeltaTime)
{
	// 스트레스가 0보다 클 때만 깎기
	if (CurrentPlayerStress > 0.0f)
	{
		// 초당 감소량에 DeltaTime을 곱해서 빼기
		CurrentPlayerStress -= StressDecayRate * DeltaTime;
		CurrentPlayerStress = FMath::Max(0.0f, CurrentPlayerStress);
	}
}

float UDirectorDataSubsystem::GetNormalizedStress() const
{
	return CurrentPlayerStress / 100.0f;
}
