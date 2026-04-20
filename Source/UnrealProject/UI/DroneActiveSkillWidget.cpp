// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneActiveSkillWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "../Component/DroneComponent.h"

void UDroneActiveSkillWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CooldownImage && CooldownImage->GetDynamicMaterial())
	{
		DynamicMaterial = CooldownImage->GetDynamicMaterial();
	}
}

void UDroneActiveSkillWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DroneCompRef && DynamicMaterial)
	{
		float CooldownRatio = DroneCompRef->GetCooldownRatio();

		DynamicMaterial->SetScalarParameterValue(TEXT("Progress"), CooldownRatio);

	}
}

void UDroneActiveSkillWidget::SetDroneComponent(UDroneComponent* InDroneComp)
{
	DroneCompRef = InDroneComp;
}
