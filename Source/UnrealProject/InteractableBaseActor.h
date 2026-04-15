// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "interface/Interactable.h"
#include "InteractableBaseActor.generated.h"

UCLASS()
class UNREALPROJECT_API AInteractableBaseActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableBaseActor();

	virtual FText GetInteractText_Implementation() override;
	virtual FText GetFeedbackText_Implementation() override;
	virtual EFeedbackType GetFeedbackType_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UInteractionComponent* InteractionComponent;

};
