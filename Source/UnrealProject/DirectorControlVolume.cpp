// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectorControlVolume.h"
#include "UnrealProjectGameMode.h"
#include "DirectorDataSubsystem.h"
#include "Components/BoxComponent.h"
#include "UnrealProject.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADirectorControlVolume::ADirectorControlVolume()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap); // 플레이어만 감지

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADirectorControlVolume::OnZoneEnter);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADirectorControlVolume::OnZoneExit);

}

void ADirectorControlVolume::OnZoneEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0)) return;

	AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	switch (ZoneType) {
	case EZoneType::SafeZone:
	{
		GM->ChangePhase(EDirectorPhase::Relax);
		GM->bIsDirectorPaused = true;

		UDirectorDataSubsystem* DataSubsystem = GetWorld()->GetSubsystem<UDirectorDataSubsystem>();
		DataSubsystem->CurrentPlayerStress = 0.f;
		break;
	}
	case EZoneType::ChokePoint:
	{
		GM->bCanEnterPeak = false;
		break;
	}
	case EZoneType::Arena:
	{
		GM->ChangePhase(EDirectorPhase::Peak);
		GM->bCanEnterFadeOut = false;
		break;
	}

	default:
		break;

	}
	
}

void ADirectorControlVolume::OnZoneExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0)) return;

	AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	switch (ZoneType) {
	case EZoneType::SafeZone:
	{
		GM->bIsDirectorPaused = false;
		break;
	}
	case EZoneType::ChokePoint:
	{
		GM->bCanEnterPeak = true;
		break;
	}
	case EZoneType::Arena:
	{
		GM->bCanEnterFadeOut = true;
		break;
	}

	default:
		break;
	}
}

