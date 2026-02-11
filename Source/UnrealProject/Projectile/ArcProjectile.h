// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealProjectProjectile.h"
#include "ArcProjectile.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AArcProjectile : public AUnrealProjectProjectile
{
	GENERATED_BODY()

public:
	virtual void LaunchTowards(FVector StartLoc, AActor* TargetActor) override;
	
};
