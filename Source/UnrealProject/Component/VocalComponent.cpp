// Fill out your copyright notice in the Description page of Project Settings.


#include "VocalComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UVocalComponent::UVocalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVocalComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UVocalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UVocalComponent::SetVocalState(EVocalState NewState)
{
	if (CurrentState == NewState) return;

	// 기존 사운드 페이드 아웃 후 정지
	if (CurrentLoopingAudio && CurrentLoopingAudio->IsPlaying())
	{
		CurrentLoopingAudio->FadeOut(0.5f, 0.0f);
	}

	CurrentState = NewState;

	if (StateSoundMap.Contains(NewState) && StateSoundMap[NewState]) {
		CurrentLoopingAudio = UGameplayStatics::SpawnSoundAttached(
			StateSoundMap[NewState],
			GetOwner()->GetRootComponent(),
			NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
			true
		);

		if (CurrentLoopingAudio) {
			CurrentLoopingAudio->FadeIn(0.5f, 1.0f);
		}
	}
}

void UVocalComponent::PlayVocalEvent(EVocalEvent Event)
{
	if (EventSoundMap.Contains(Event)) {
		UGameplayStatics::PlaySoundAtLocation(this, EventSoundMap[Event], GetOwner()->GetActorLocation());

		if (Event == EVocalEvent::EVE_Death && CurrentLoopingAudio)
		{
			CurrentLoopingAudio->Stop();
		}
	}
}
