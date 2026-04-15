// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableBaseActor.h"
#include "Component/InteractionComponent.h"

// Sets default values
AInteractableBaseActor::AInteractableBaseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComp"));

}

FText AInteractableBaseActor::GetInteractText_Implementation()
{
	if (UInteractionComponent* Comp = FindComponentByClass<UInteractionComponent>())
	{
		return Comp->InteractPromptText;
	}
	return FText::GetEmpty();
}

FText AInteractableBaseActor::GetFeedbackText_Implementation()
{
	if (UInteractionComponent* Comp = FindComponentByClass<UInteractionComponent>())
	{
		return Comp->FeedbackText;
	}
	return FText::GetEmpty();
}

EFeedbackType AInteractableBaseActor::GetFeedbackType_Implementation()
{
	if (UInteractionComponent* Comp = FindComponentByClass<UInteractionComponent>())
	{
		return Comp->FeedbackType;
	}
	return EFeedbackType::Neutral;
}
