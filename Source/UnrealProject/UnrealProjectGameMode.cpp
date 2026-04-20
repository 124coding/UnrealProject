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

void AUnrealProjectGameMode::ResetDirectorState()
{
	// FSM 상태 초기화
	CurrentPhase = EDirectorPhase::Relax;
	TimeInCurrentPhase = 0.0f;
	SpawnCooldown = 0.0f;

	// 토큰 및 경제 시스템 초기화
	CurrentDirectorTokens = 0;

	// 통제 플래그(Flag) 초기화
	bIsInSafeZone = false;
	bCanEnterPeak = true;
	bCanEnterFadeOut = true;

	// 공간 동기화 초기화
	CurrentActiveGroupID = 0;

	// 맵에 남아있는 모든 적 퇴근
	for (const auto& Pair : EnemyPoolMap)
	{
		UObjectPoolComponent* EnemyPool = Pair.Value;
		if (EnemyPool)
		{
			EnemyPool->ReturnAllToPool();
		}
	}

	// 날아가고 있던 모든 적의 총알 일괄 회수
	for (const auto& Pair : EnemyProjectilePoolMap)
	{
		UObjectPoolComponent* ProjectilePool = Pair.Value;
		if (ProjectilePool)
		{
			ProjectilePool->ReturnAllToPool();
		}
	}

	ActiveEnemyCount = 0;
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

void AUnrealProjectGameMode::GameCleared(AController* Controller)
{
	if (Controller && Controller->IsLocalPlayerController()) {

		if (AUnrealProjectPlayerController* PlayerController = Cast<AUnrealProjectPlayerController>(Controller)) {

			PlayerController->OnGameCleared();
		}

	}
}

void AUnrealProjectGameMode::SpendTokensToSpawn(int32 MaxTokensToSpend, const TArray<FEnemySpawnInfo>& SpawnList, AActor* TargetActor)
{
	int32 TokensLeft = MaxTokensToSpend;
	int32 Failsafe = 50; // 무한 루프 방지용 (최대 50번만 구매 시도)

	while (TokensLeft > 0 && Failsafe > 0) {
		Failsafe--;

		// 살 수 있는 몬스터만 필터링
		TArray<FEnemySpawnInfo> AffordableEnemies;
		for (const FEnemySpawnInfo& Info : SpawnList) {
			if (Info.SpawnCost <= TokensLeft) {
				AffordableEnemies.Add(Info);
			}
		}

		// 살 수 있는 몬스터가 없다면 종료
		if (AffordableEnemies.Num() == 0) break;

		TSubclassOf<AActor> SelectedClass = GetRandomEnemyClass(AffordableEnemies);

		if (SelectedClass) {
			
			// 방금 뽑은 클래스의 비용
			int32 CostToDeduct = 0;
			for (const FEnemySpawnInfo& Info : AffordableEnemies) {
				if (Info.EnemyClass == SelectedClass) {
					CostToDeduct = Info.SpawnCost;
					break;
				}
			}

			// 비용 차감
			TokensLeft -= CostToDeduct;
			CurrentDirectorTokens -= CostToDeduct;

			SpawnEnemyInGroup(CurrentActiveGroupID, 1, SelectedClass, TargetActor);
		}
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
	if (NewPhase == EDirectorPhase::Peak && !bCanEnterPeak) return;

	CurrentPhase = NewPhase;
	TimeInCurrentPhase = 0.0f; // 페이즈가 바뀔 때마다 머문 시간 초기화
	SpawnCooldown = 0.0f;      // 스폰 쿨타임도 초기화

	OnPhaseChanged.Broadcast(CurrentPhase);
}

void AUnrealProjectGameMode::DirectorUpdateLoop()
{

	UDirectorDataSubsystem* DataSubsystem = GetWorld()->GetSubsystem<UDirectorDataSubsystem>();
	if (!DataSubsystem) return;

	float DirectorInterval = 0.5f; // 타이머 주기와 동일
	TimeInCurrentPhase += DirectorInterval;

	float CurrentStress = DataSubsystem->GetNormalizedStress(); // 0.0 ~ 1.0

	if (bIsInSafeZone) {
		DataSubsystem->DecayStress(DirectorInterval);
		return;
	}

	// 타겟 지정용 플레이어
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);

	// 매 틱마다 자금을 모으기 위함
	CurrentDirectorTokens += (TokenGenerationRate * DirectorInterval);


	/*if (CurrentPhase == EDirectorPhase::Peak && CurrentStress >= 0.95f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Warning! Wave Change"));
		ChangePhase(EDirectorPhase::BuildUp);
		return;
	}*/

	switch (CurrentPhase) {
	case EDirectorPhase::Relax:

		// Stress 감소
		DataSubsystem->DecayStress(DirectorInterval);

		/*if (TimeInCurrentPhase > MaxRelaxTime)
		{
			ChangePhase(EDirectorPhase::BuildUp);
		}*/

		if (CurrentStress >= 0.2f)
		{

			ChangePhase(EDirectorPhase::BuildUp);
		}
		break;

	case EDirectorPhase::BuildUp:
	{
		// 산발적 스폰
		SpawnCooldown -= DirectorInterval;

		if (SpawnCooldown <= 0.0f)
		{
			//if (NormalEnemyList.Num() > 0)
			//{
			//	// 랜덤 뽑기 함수
			//	TSubclassOf<AActor> SelectedClass = GetRandomEnemyClass(NormalEnemyList);

			//	// 뽑힌 랜덤 클래스로 스폰
			//	if (SelectedClass) {
			//		SpawnEnemyInGroup(CurrentActiveGroupID, FMath::RandRange(2, 3), SelectedClass/*, PlayerActor*/);
			//	}
			//}

			// 현재 자금의 10프로만 5초마다 사용해서 플레이어를 공격
			int32 TokensToSpend = FMath::Max(3.f, CurrentDirectorTokens * 0.1f);
			SpendTokensToSpawn(TokensToSpend, NormalEnemyList, PlayerActor);
			SpawnCooldown = 3.0f;
		}

		// 플레이어의 스트레스가 1분동안 어느 수준으로 안올라가면 강제 Peak 
		bool bTooBored = (CurrentStress < 0.3f && TimeInCurrentPhase > 60.0f);

		// 플레이어의 스트레스가 80%를 넘거나, 빌드업 시간이 40초를 넘으면 Peak
		if (CurrentStress >= 0.7f || bTooBored /*|| TimeInCurrentPhase > 40.0f*/)
		{
			ChangePhase(EDirectorPhase::Peak);
		}
		break;
	}

	case EDirectorPhase::Peak:

		SpawnCooldown -= DirectorInterval;

		// 1초마다 적을 우루루 쏟아냅니다.
		if (SpawnCooldown <= 0.0f)
		{
			if (CurrentDirectorTokens > 0)
			{
				// 남은 돈의 20프로씩만 사용
				// (최소 10토큰은 보장해서 조금씩 나오는 걸 방지)
				// (최대 200토큰을 고정)
				/* 최대, 최소 토큰을 에디터에서 수정가능하도록 할 수도 있음 */
				int32 TokensToSpendThisTick = FMath::Max(10, FMath::Min(200, FMath::RoundToInt(CurrentDirectorTokens * 0.2f)));

				// 만약 가진 자금이 10보다 적으면 남은 돈 전부 올인
				if (CurrentDirectorTokens < 10) TokensToSpendThisTick = CurrentDirectorTokens;

				SpendTokensToSpawn(TokensToSpendThisTick, HordeEnemyList, PlayerActor);
			}
			SpawnCooldown = 1.0f; // 1초마다 적을 스폰
		}

		if (TimeInCurrentPhase > PeakDuration && bCanEnterFadeOut)
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

TSubclassOf<AActor> AUnrealProjectGameMode::GetRandomEnemyClass(const TArray<FEnemySpawnInfo>& SpawnList)
{
	if (SpawnList.Num() == 0) return nullptr;

	// 1모든 가중치의 총합
	float TotalWeight = 0.0f;
	for (const FEnemySpawnInfo& Info : SpawnList)
	{
		TotalWeight += Info.SpawnWeight;
	}

	if (TotalWeight <= 0.0f) return nullptr;

	float Random = FMath::FRandRange(0.0f, TotalWeight);

	for (const FEnemySpawnInfo& Info : SpawnList)
	{
		Random -= Info.SpawnWeight;
		if (Random <= 0.0f)
		{
			return Info.EnemyClass;
		}
	}

	return SpawnList.Last().EnemyClass;
}

int32 AUnrealProjectGameMode::GetAliveEnemyCount() const
{
	return ActiveEnemyCount;
}

int32 AUnrealProjectGameMode::GetCurrentTokens() const
{
	return CurrentDirectorTokens;
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
