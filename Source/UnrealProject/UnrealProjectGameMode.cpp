// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectGameMode.h"
#include "Character/UnrealProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AUnrealProjectGameMode::AUnrealProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AUnrealProjectGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	AUnrealProjectCharacter* PlayerChar = Cast<AUnrealProjectCharacter>(NewPlayer->GetPawn());

	if (PlayerChar) {
		PlayerChar->OnDeath.AddUniqueDynamic(this, &AUnrealProjectGameMode::OnMyPawnDied);
	}
}

void AUnrealProjectGameMode::OnMyPawnDied(AActor* Victim, AActor* Killer)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Died! Respawning in 5 seconds..."));

	AController* VictimController = Cast<APawn>(Victim)->GetController();

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AUnrealProjectGameMode::RespawnPlayer, VictimController);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);
}

void AUnrealProjectGameMode::RespawnPlayer(AController* Controller)
{
	if (Controller && Controller->IsPlayerController())
	{
		// 게임 재시작
		UWorld* World = GetWorld();
		if (World)
		{
			// 현재 레벨의 이름 가져오기
			FName CurrentLevelName = FName(*World->GetName());

			// 해당 레벨을 다시 열기
			UGameplayStatics::OpenLevel(World, CurrentLevelName);
		}
		
		// 플레이어 리스폰
		/*if(APawn* OldPawn = Controller->GetPawn())
		{
			Controller->UnPossess();
			OldPawn->Destroy(); 
		}

		UE_LOG(LogTemp, Log, TEXT("RESTART_GAME"));
		RestartPlayer(Controller);*/
	}
}
