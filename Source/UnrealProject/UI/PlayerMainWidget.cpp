// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMainWidget.h"
#include "DroneMainWidget.h"

void UPlayerMainWidget::InitializeHUD(UDroneComponent* DroneComp)
{
	if (WBP_DroneMainPanel) {
		WBP_DroneMainPanel->InitializeDroneUI(DroneComp);
	}
}
