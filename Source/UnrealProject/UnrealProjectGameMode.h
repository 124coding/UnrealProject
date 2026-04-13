// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnumTypes/DirectorTypes.h"
#include "UnrealProjectGameMode.generated.h"

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	// 스폰할 적 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> EnemyClass;

	// 등장 확률 (가중치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnWeight = 1.0f;

	// 몬스터 스폰 시 지불 비용
	UPROPERTY(EditAnywhere)
	int32 SpawnCost = 1;
};

UCLASS(minimalapi)
class AUnrealProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUnrealProjectGameMode();

	// 디렉터 상태 초기화
	UFUNCTION(BlueprintCallable, Category = "Director")
	void ResetDirectorState();

protected:
	virtual void BeginPlay() override;

	// 재시작 함수
	virtual void RestartPlayer(AController* NewPlayer) override;

	// 실제 플레이어 폰 사망 시 실행 될 함수
	UFUNCTION()
	void OnMyPawnDied(AActor* Victim, AActor* Killer);

	// 타이머가 끝나면 실행될 부활 함수
	void RespawnPlayer(AController* Controller);

public:
	UFUNCTION(BlueprintCallable)
	void GameCleared(AController* Controller);

protected:
	// 적 종류(class)를 키(key)로 넣으면, 해당 적을 담고 있는 풀을 주는 맵
	UPROPERTY()
	TMap<UClass*, class UObjectPoolComponent*> EnemyPoolMap;

	// 게임 시작 시 만들고 싶은 적의 종류와 개수 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Setup")
	TMap<TSubclassOf<AActor>, int32> InitialEnemyPoolConfig;

	// 맵에 있는 모든 볼륨을 저장할 배열
	UPROPERTY()
	TArray<class ASpawnVolume*> AllSpawnVolumes;

	// 디렉터의 현재 보유 잔팅(토큰)
	UPROPERTY(VisibleAnywhere, Category = "Director")
	int32 CurrentDirectorTokens = 0;

	// 1초당 디렉터가 벌어들이는 토큰 양 (시간이 지날수록 웨이브가 거세짐)
	UPROPERTY(EditDefaultsOnly, Category = "Director")
	int32 TokenGenerationRate = 3;

	// 토큰 기반 스폰 함수 선언
	void SpendTokensToSpawn(int32 MaxTokensToSpend, const TArray<FEnemySpawnInfo>& SpawnList, AActor* TargetActor);

protected:

	// 적의 총알 종류를 담고 있는 맵
	UPROPERTY()
	TMap<UClass*, class UObjectPoolComponent*> EnemyProjectilePoolMap;

	// 게임 시작 시 만들고 싶은 적의 총알의 종류와 개수 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Setup")
	TMap<TSubclassOf<AActor>, int32> InitialEnemyProjectilePoolConfig;

	// 디렉터 스폰 클래스 설정
	UPROPERTY(EditDefaultsOnly, Category = "Director|Classes")
	TArray<FEnemySpawnInfo> NormalEnemyList; // 빌드업 때 나올 일반 적들

	UPROPERTY(EditDefaultsOnly, Category = "Director|Classes")
	TArray<FEnemySpawnInfo> HordeEnemyList;  // 피크 때 쏟아질 특수/물량 적들

	// 디렉터 밸런스 설정
	//UPROPERTY(EditDefaultsOnly, Category = "Director|Balance")
	//float MaxRelaxTime = 30.0f; // 휴식기 최대 시간

	UPROPERTY(EditDefaultsOnly, Category = "Director|Balance")
	float PeakDuration = 15.0f; // 웨이브 지속 시간

	UPROPERTY(EditDefaultsOnly, Category = "Director|Balance")
	int32 PeakSpawnCount = 10;  // 웨이브 때 한 번에 스폰할 마릿수

	FTimerHandle DirectorTimerHandle;

	float TimeInCurrentPhase = 0.0f; // 현재 페이즈에 머문 시간
	float SpawnCooldown = 0.0f;      // 스폰 간격 조절용 쿨타임

	void DirectorUpdateLoop();

private:
	TSubclassOf<AActor> GetRandomEnemyClass(const TArray<FEnemySpawnInfo>& SpawnList);

public:
	// 안전지대이기인지 확인
	bool bIsInSafeZone = false;
	
	// Peak에 들어가도 되는지 확인
	bool bCanEnterPeak = true;

	// FadeOut에 들어가도 되는지 확인
	bool bCanEnterFadeOut = true;

	// 현재 페이즈
	EDirectorPhase CurrentPhase = EDirectorPhase::Relax;

	// 현재 맵에 살아있는(활성화된) 적의 총 마릿수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	int32 ActiveEnemyCount = 0;

	// 디렉터 루프에서 호출할 Getter 함수
	int32 GetAliveEnemyCount() const;

	int32 GetCurrentTokens() const;

	AActor* SpawnProjectileFromPool(TSubclassOf<AActor> ProjectileClass, FVector Location, FRotator Rotation);

	// 트리거가 밟힐 때 호출할 함수
	UFUNCTION(BlueprintCallable)
	void SetActiveSpawnGroup(int32 NewGroupID);

	void ChangePhase(EDirectorPhase NewPhase);

public:
	// 디렉터가 현재 스폰 타겟으로 삼을 그룹 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Director")
	int32 CurrentActiveGroupID = 0;

	// 외부에서 호출하는 통합 스폰 함수
	UFUNCTION(BlueprintCallable)
	AActor* SpawnEnemyFromPool(TSubclassOf<AActor> EnemyClass, FVector Location);

	// 특정 그룹 ID에서 스폰시키는 함수
	UFUNCTION(BlueprintCallable)
	void SpawnEnemyInGroup(int32 TargetGroupID, int32 SpawnCount, TSubclassOf<AActor> EnemyClassToSpawn, AActor* AttackTarget = nullptr);
};



