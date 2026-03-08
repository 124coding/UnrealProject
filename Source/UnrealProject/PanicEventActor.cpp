// Fill out your copyright notice in the Description page of Project Settings.


#include "PanicEventActor.h"
#include "DirectorDataSubsystem.h"
#include "UnrealProjectGameMode.h"

// Sets default values
APanicEventActor::APanicEventActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APanicEventActor::BeginPlay()
{
	Super::BeginPlay();

}

void APanicEventActor::Interact_Implementation(AActor* InstigatorActor)
{
	if (!bInteractTriggered) return;

	ExecutePanicEvent();
}

void APanicEventActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (!bHitTriggered) return;

	ExecutePanicEvent();
}

void APanicEventActor::ExecutePanicEvent()
{
	if (bHasBeenTriggered) return;
	bHasBeenTriggered = true;

	// 시각, 청각 연출
	// PlayPanicEffects();

	// 스트레스 수치 최대로
	if (UDirectorDataSubsystem* DataSubsystem = GetWorld()->GetSubsystem<UDirectorDataSubsystem>())
	{
		DataSubsystem->AddStressEvent(100.0f);
	}

	// GameMode를 강제로 Peak로 함
	if (AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ChangePhase(EDirectorPhase::Peak);
		UE_LOG(LogTemp, Warning, TEXT("Panic Event"));
	}
}

