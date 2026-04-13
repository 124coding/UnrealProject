// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VocalComponent.generated.h"

// 아래의 FSM 방식의 Vocal Sound 출력은 
// 몹이 더 많거나 더 다양한 소리가 존재해야 할 경우 부적합할 수 있어
// 언리얼의 GameplayTag를 이용하는 방식으로 수정 가능

// 지속적
UENUM(BlueprintType)
enum class EVocalState : uint8
{
	EVS_None,
	EVS_Idle,
	EVS_Jogging,
	EVS_Sprinting,
	EVS_Downed
};

// 단발적
UENUM(BlueprintType)
enum class EVocalEvent : uint8 { 
	EVE_None,
	EVE_Spawn,
	EVE_Death,
	EVE_Damaged
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UVocalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVocalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "Vocal")
	TMap<EVocalState, USoundBase*> StateSoundMap;

	UPROPERTY(EditDefaultsOnly, Category = "Vocal")
	TMap<EVocalEvent, USoundBase*> EventSoundMap;

	UPROPERTY()
	class UAudioComponent* CurrentLoopingAudio; // 루핑 사운드 관리용

	EVocalState CurrentState = EVocalState::EVS_None;

	// 지속되는 소리 변경 (상태 전환용)
	void SetVocalState(EVocalState NewState);

	// 단발성 소리 재생 (사건 발생용)
	void PlayVocalEvent(EVocalEvent Event);
		
};
