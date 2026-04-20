// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneMainWidget.h"
#include "DroneActiveSkillWidget.h"

void UDroneMainWidget::InitializeDroneUI(UDroneComponent* DroneComp)
{
	if (WBP_SkillWidget)
	{
		WBP_SkillWidget->SetDroneComponent(DroneComp);
	}
}
