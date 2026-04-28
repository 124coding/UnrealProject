// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "EnumTypes/DirectorTypes.h"
#include "PoolState.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDirectorPhaseChanged, EDirectorPhase, NewPhase);

UCLASS()
class UNREALPROJECT_API APoolState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APoolState();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pool")
	class UObjectPoolComponent* DamageTextPool;

	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSubclassOf<AActor> DamageTextClass;

public:
	// 적 종류(class)를 키(key)로 넣으면, 해당 적을 담고 있는 풀을 주는 맵
	UPROPERTY()
	TMap<UClass*, class UObjectPoolComponent*> EnemyPoolMap;

	// 게임 시작 시 만들고 싶은 적의 종류와 개수 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Setup")
	TMap<TSubclassOf<AActor>, int32> InitialEnemyPoolConfig;

public:
	// 적의 총알 종류를 담고 있는 맵
	UPROPERTY()
	TMap<UClass*, class UObjectPoolComponent*> EnemyProjectilePoolMap;

	// 게임 시작 시 만들고 싶은 적의 총알의 종류와 개수 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Setup")
	TMap<TSubclassOf<AActor>, int32> InitialEnemyProjectilePoolConfig;

public:
	// 현재 페이즈
	EDirectorPhase CurrentPhase = EDirectorPhase::Relax;

	UPROPERTY(BlueprintAssignable, Category = "Director|Event")
	FOnDirectorPhaseChanged OnPhaseChanged;
};
