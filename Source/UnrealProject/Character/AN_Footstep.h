// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_Footstep.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UAN_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_Footstep();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
public:
	// 설정할 소리 (걷기, 달리기, 앉은 상태에서의 모든 소리를 다르게 하기 위해 따로 변수들을 설정할 필요가 있음)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundToPlay;

	// 소리의 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float NoiseLoudness = 1.0f;

	// 들을 수 있는 최대 거리 (0이면 소리의 감쇠 범위 따름)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float NoiseMaxRange = 0.0f;

	// 소리 크기 조절 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;

	// 소리 높낮이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float PitchMultiplier = 1.0f;

	// 소음 태그(다양한 소리 중 발소리를 구분할 때 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FName NoiseTag = TEXT("Footstep");
};
