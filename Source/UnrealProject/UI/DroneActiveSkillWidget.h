// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DroneActiveSkillWidget.generated.h"

/**
 * 
 */

class UDroneComponent;

UCLASS()
class UNREALPROJECT_API UDroneActiveSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void SetDroneComponent(UDroneComponent* InDroneComp);

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* CooldownImage;

private:
	UPROPERTY()
	UDroneComponent* DroneCompRef;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;
};
