// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DirectorTypes.generated.h"

UENUM(BlueprintType)
enum class EDirectorPhase : uint8
{
	Relax		UMETA(DisplayName = "Relax"),
	BuildUp		UMETA(DisplayName = "BuildUp"),
	Peak		UMETA(DisplayName = "Peak"),
	FadeOut		UMETA(DisplayName = "FadeOut"),
	None
};
