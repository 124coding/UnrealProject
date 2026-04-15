// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UITypes.generated.h"

UENUM(BlueprintType)
enum class EFeedbackType : uint8
{
    Positive    UMETA(DisplayName = "Positive"),
    Negative    UMETA(DisplayName = "Negative"),
    Neutral     UMETA(DisplayName = "Neutral")
};