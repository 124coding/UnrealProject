// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectGameMode.h"
#include "Character/UnrealProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Component/ObjectPoolComponent.h"
#include "SpawnVolume.h"
#include "Enemy/BaseEnemy.h"

AUnrealProjectGameMode::AUnrealProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AUnrealProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 설정된 적 종류만큼 풀 컴포넌트 동적 생성
	for (auto& Elem : InitialPoolConfig) {
		TSubclassOf<AActor> ClassType = Elem.Key;
		int32 Count = Elem.Value;

		if (ClassType) {
			// 새 풀 컴포넌트 생성(이름은 클래스 이름)
			FString PoolName = FString::Printf(TEXT("Pool_%s"), *ClassType->GetName());
			UObjectPoolComponent* NewPool = NewObject<UObjectPoolComponent>(this, FName(*PoolName));

			// 컴포넌트 등록
			NewPool->RegisterComponent();

			// 풀 초기화
			NewPool->InitializePool(ClassType, Count);

			// 맵에 저장
			EnemyPoolMap.Add(ClassType, NewPool);
		}
	}

	// 맵의 모든 스폰 볼륨을 찾아서 캐싱
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors) {
		if (ASpawnVolume* Vol = Cast<ASpawnVolume>(Actor)) {
			AllSpawnVolumes.Add(Vol);
		}
	}
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

void AUnrealProjectGameMode::SpawnEnemyInGroup(int32 TargetGroupID, int32 SpawnCount, TSubclassOf<AActor> EnemyClassToSpawn, AActor* AttackTarget)
{
	// 해당 ID를 가진 볼륨들만 임시로 모으기
	TArray<ASpawnVolume*> TargetVolumes;

	for (ASpawnVolume* Vol : AllSpawnVolumes) {
		if (Vol && Vol->SpawnGroupID == TargetGroupID) {
			TargetVolumes.Add(Vol);
		}
	}

	// 만약 해당 그룹이 하나도 없으면 리턴
	if (TargetVolumes.Num() == 0) return;

	// 스폰 시작
	for (int32 i = 0; i < SpawnCount; i++) {
		int32 RandIndex = FMath::RandRange(0, TargetVolumes.Num() - 1);
		ASpawnVolume* SelectedVol = TargetVolumes[RandIndex];

		FVector SpawnLoc = SelectedVol->GetRandomPointInVolume();
		AActor* SpawnedActor = SpawnEnemyFromPool(EnemyClassToSpawn, SpawnLoc);

		// 타겟이 지정되어 있으면 BaseEnemy의 타겟 지정 함수 실행
		if (SpawnedActor && AttackTarget) {
			if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(SpawnedActor)) {
				Enemy->SetCommandTarget(AttackTarget);
			}
		}
	}
}

AActor* AUnrealProjectGameMode::SpawnEnemyFromPool(TSubclassOf<AActor> EnemyClass, FVector Location)
{
	if (UObjectPoolComponent** FoundPool = EnemyPoolMap.Find(EnemyClass)) {
		UE_LOG(LogTemp, Warning, TEXT("Spawn this type of class: %s"), *EnemyClass->GetName());
		return (*FoundPool)->SpawnFromPool(Location, FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Warning, TEXT("Pool don't have this type of class: %s"), *EnemyClass->GetName());

	return nullptr;
}
