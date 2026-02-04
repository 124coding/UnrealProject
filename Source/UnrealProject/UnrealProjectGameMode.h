// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UnrealProjectGameMode.generated.h"

UCLASS(minimalapi)
class AUnrealProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUnrealProjectGameMode();

protected:
	virtual void BeginPlay() override;

	// 재시작 함수
	virtual void RestartPlayer(AController* NewPlayer) override;

	// 실제 플레이어 폰 사망 시 실행 될 함수
	UFUNCTION()
	void OnMyPawnDied(AActor* Victim, AActor* Killer);

	// 타이머가 끝나면 실행될 부활 함수
	void RespawnPlayer(AController* Controller);

protected:
	// 적 종류(class)를 키(key)로 넣으면, 해당 적을 담고 있는 풀을 주는 맵
	UPROPERTY()
	TMap<UClass*, class UObjectPoolComponent*> EnemyPoolMap;

	// 게임 시작 시 만들고 싶은 적의 종류와 개수 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Setup")
	TMap<TSubclassOf<AActor>, int32> InitialPoolConfig;

	// 맵에 있는 모든 볼륨을 저장할 배열
	UPROPERTY()
	TArray<class ASpawnVolume*> AllSpawnVolumes;

public:
	// 외부에서 호출하는 통합 스폰 함수
	UFUNCTION(BlueprintCallable)
	AActor* SpawnEnemyFromPool(TSubclassOf<AActor> EnemyClass, FVector Location);

	// 특정 그룹 ID에서 스폰시키는 함수
	UFUNCTION(BlueprintCallable)
	void SpawnEnemyInGroup(int32 TargetGroupID, int32 SpawnCount, TSubclassOf<AActor> EnemyClassToSpawn);
};



