// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../EnumTypes/UITypes.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNREALPROJECT_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InstigatorActor);

	// 화면에 띄울 텍스트를 물어볼 때 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractText();

	// 상호작용 후에 호출할 텍스트를 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetFeedbackText();

	// 상호작용 후에 호출할 애니메이션의 타입
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	EFeedbackType GetFeedbackType();
};
