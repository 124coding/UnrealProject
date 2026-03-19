// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DirectorDataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UDirectorDataSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// 플레이어의 현재 스트레스 수치 (0.0 ~ 100.0)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	float CurrentPlayerStress = 0.0f;

	// 1초당 감소하는 스트레스 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director|Stress")
	float StressDecayRate = 5.0f;

	// 외부에서 이벤트를 통해 스트레스를 증가시키는 함수
	UFUNCTION(BlueprintCallable)
	void AddStressEvent(float StressAmount);

	// 매 프레임(또는 타이머)마다 스트레스를 서서히 감소시키는 함수 (휴식기용)
	UFUNCTION(BlueprintCallable)
	void DecayStress(float DeltaTime);

	// 현재 스트레스 레벨 반환
	float GetNormalizedStress() const;
};
