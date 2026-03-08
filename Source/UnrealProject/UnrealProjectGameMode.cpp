// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectGameMode.h"
#include "Character/UnrealProjectPlayerController.h"
#include "Character/UnrealProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Component/ObjectPoolComponent.h"
#include "SpawnVolume.h"
#include "Enemy/BaseEnemy.h"
#include "DirectorDataSubsystem.h"

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
	for (auto& Elem : InitialEnemyPoolConfig) {
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

	for (auto& Elem : InitialEnemyProjectilePoolConfig) {
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
			EnemyProjectilePoolMap.Add(ClassType, NewPool);
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

	GetWorld()->GetTimerManager().SetTimer(
		DirectorTimerHandle,
		this,
		&AUnrealProjectGameMode::DirectorUpdateLoop,
		0.5f,
		true
	);
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
	// UE_LOG(LogTemp, Warning, TEXT("Player Died! Respawning in 5 seconds..."));

	AController* VictimController = Cast<APawn>(Victim)->GetController();
	if (!VictimController) {
		return;
	}

	if (AUnrealProjectPlayerController* PlayerController = Cast<AUnrealProjectPlayerController>(VictimController)) {

		FTimerHandle TimerHandle;

		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			PlayerController,
			&AUnrealProjectPlayerController::ShowGameOverUI,
			3.0f,
			false
		);
	}

	/*FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AUnrealProjectGameMode::RespawnPlayer, VictimController);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);*/
}

void AUnrealProjectGameMode::RespawnPlayer(AController* Controller)
{
	if (Controller && Controller->IsPlayerController())
	{
		// 게임 재시작
		//UWorld* World = GetWorld();
		//if (World)
		//{
		//	// 현재 레벨의 이름 가져오기
		//	FName CurrentLevelName = FName(*World->GetName());

		//	// 해당 레벨을 다시 열기
		//	UGameplayStatics::OpenLevel(World, CurrentLevelName);
		//}
		
		// 플레이어 리스폰
		if(APawn* OldPawn = Controller->GetPawn())
		{
			Controller->UnPossess();
			OldPawn->Destroy(); 
		}

		UE_LOG(LogTemp, Log, TEXT("RESTART_GAME"));
		RestartPlayer(Controller);
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

void AUnrealProjectGameMode::ChangePhase(EDirectorPhase NewPhase)
{
	CurrentPhase = NewPhase;
	TimeInCurrentPhase = 0.0f; // 페이즈가 바뀔 때마다 머문 시간 초기화
	SpawnCooldown = 0.0f;      // 스폰 쿨타임도 초기화
}

void AUnrealProjectGameMode::DirectorUpdateLoop()
{
	UDirectorDataSubsystem* DataSubsystem = GetWorld()->GetSubsystem<UDirectorDataSubsystem>();
	if (!DataSubsystem) return;

	float DirectorInterval = 0.5f; // 타이머 주기와 동일
	TimeInCurrentPhase += DirectorInterval;

	float CurrentStress = DataSubsystem->GetNormalizedStress(); // 0.0 ~ 1.0

	// 타겟 지정용 플레이어
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);

	switch (CurrentPhase) {
	case EDirectorPhase::Relax:

		// Stress 감소
		DataSubsystem->DecayStress(DirectorInterval);

		if (TimeInCurrentPhase > MaxRelaxTime)
		{
			ChangePhase(EDirectorPhase::BuildUp);
		}
		break;

	case EDirectorPhase::BuildUp:

		// 산발적 스폰
		SpawnCooldown -= DirectorInterval;

		if (SpawnCooldown <= 0.0f)
		{
			if (NormalEnemyClasses.Num() > 0)
			{
				// 0번 인덱스부터 배열의 마지막 인덱스 사이에서 랜덤 뽑기
				int32 RandomIndex = FMath::RandRange(0, NormalEnemyClasses.Num() - 1);
				TSubclassOf<AActor> SelectedClass = NormalEnemyClasses[RandomIndex];

				// 뽑힌 랜덤 클래스로 스폰
				SpawnEnemyInGroup(CurrentActiveGroupID, FMath::RandRange(2, 3), SelectedClass, PlayerActor);
			}
			SpawnCooldown = 5.0f;
		}

		// 플레이어의 스트레스가 80%를 넘거나, 빌드업 시간이 40초를 넘으면 Peak
		if (CurrentStress >= 0.8f || TimeInCurrentPhase > 40.0f)
		{
			ChangePhase(EDirectorPhase::Peak);
		}
		break;

	case EDirectorPhase::Peak:

		SpawnCooldown -= DirectorInterval;
		if (SpawnCooldown <= 0.0f)
		{
			if (HordeEnemyClasses.Num() > 0)
			{
				// 한 번에 지정된 숫자(PeakSpawnCount)만큼 스폰하되, 
				// 매번 1마리씩 다른 종류를 뽑아서 호출
				for (int32 i = 0; i < PeakSpawnCount; i++)
				{
					int32 RandomIndex = FMath::RandRange(0, HordeEnemyClasses.Num() - 1);
					TSubclassOf<AActor> SelectedClass = HordeEnemyClasses[RandomIndex];

					// 1마리씩 개별 스폰
					SpawnEnemyInGroup(CurrentActiveGroupID, 1, SelectedClass, PlayerActor);
				}
			}
			SpawnCooldown = 3.0f;
		}

		if (TimeInCurrentPhase > PeakDuration)
		{
			ChangePhase(EDirectorPhase::FadeOut);
		}
		break;

	case EDirectorPhase::FadeOut:

		int32 AliveEnemies = GetAliveEnemyCount();

		// 적을 다 잡았거나, 너무 오래 지연(30초)되면 강제로 휴식기
		if (AliveEnemies <= 0 || TimeInCurrentPhase > 30.0f)
		{
			DataSubsystem->CurrentPlayerStress = 0.0f; // 스트레스 초기화
			ChangePhase(EDirectorPhase::Relax);
		}
		break;
	}
}

int32 AUnrealProjectGameMode::GetAliveEnemyCount() const
{
	return ActiveEnemyCount;
}

AActor* AUnrealProjectGameMode::SpawnProjectileFromPool(TSubclassOf<AActor> ProjectileClass, FVector Location, FRotator Rotation)
{
	if (!IsValid(ProjectileClass)) {
		return nullptr;
	}

	if (!EnemyProjectilePoolMap.Contains(ProjectileClass)) {
		// 풀이 없다면 새로 생성해서 TMap에 등록
		UObjectPoolComponent* NewPool = NewObject<UObjectPoolComponent>(this);

		NewPool->RegisterComponent();

		NewPool->InitializePool(ProjectileClass, 50); // 50개 정도 생성
		EnemyProjectilePoolMap.Add(ProjectileClass, NewPool);
	}

	UObjectPoolComponent* TargetPool = EnemyProjectilePoolMap[ProjectileClass];
	AActor* Projectile = TargetPool->SpawnFromPool(Location, Rotation);

	return Projectile;
}

void AUnrealProjectGameMode::SetActiveSpawnGroup(int32 NewGroupID)
{
	CurrentActiveGroupID = NewGroupID;
	/* 여기서 새 구역 진입 UI 알림 등을 띄우기도 가능 (델리게이트 식)*/
}

AActor* AUnrealProjectGameMode::SpawnEnemyFromPool(TSubclassOf<AActor> EnemyClass, FVector Location)
{
	if (UObjectPoolComponent** FoundPool = EnemyPoolMap.Find(EnemyClass)) {
		UE_LOG(LogTemp, Warning, TEXT("Spawn this type of class: %s"), *EnemyClass->GetName());
		ActiveEnemyCount++;
		return (*FoundPool)->SpawnFromPool(Location, FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Warning, TEXT("Pool don't have this type of class: %s"), *EnemyClass->GetName());

	return nullptr;
}
