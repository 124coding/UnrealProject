// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DroneMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UDroneMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UDroneActiveSkillWidget* WBP_SkillWidget;

public:
	void InitializeDroneUI(class UDroneComponent* DroneComp);
};
