// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UPlayerMainWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UDroneMainWidget* WBP_DroneMainPanel;
	
public:
	void InitializeHUD(class UDroneComponent* DroneComp);
};
