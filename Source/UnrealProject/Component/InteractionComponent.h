// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../EnumTypes/UITypes.h"
#include "InteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractPromptText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText FeedbackText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EFeedbackType FeedbackType = EFeedbackType::Neutral;
};
