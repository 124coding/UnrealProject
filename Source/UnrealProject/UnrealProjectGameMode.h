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
	// 재시작 함수
	virtual void RestartPlayer(AController* NewPlayer) override;

	// 실제 플레이어 폰 사망 시 실행 될 함수
	UFUNCTION()
	void OnMyPawnDied(AActor* Victim, AActor* Killer);

	// 타이머가 끝나면 실행될 부활 함수
	void RespawnPlayer(AController* Controller);
};



