// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SystemSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API USystemSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USystemSaveGame();


public:
	// Audio
		
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MasterVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float BGMVolume;
	
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float SFXVolume;

public:
	// Gameplay

	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	float MouseSensitivity;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Video")
	FIntPoint Resolution;
};
