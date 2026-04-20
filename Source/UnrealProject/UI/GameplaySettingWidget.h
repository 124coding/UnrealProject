// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplaySettingWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UGameplaySettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
protected:
	// 감도 조절 슬라이더
	UPROPERTY(meta = (BindWidget))
	class USlider* Slider_MouseSens;

private:
	UFUNCTION()
	void OnMouseSensChanged(float Value);

	UPROPERTY()
	class USystemSaveGame* SaveData;

public:
	// 메인에서 부를 초기화 함수
	void InitGameplaySettings(class USystemSaveGame* InSettings);
};
