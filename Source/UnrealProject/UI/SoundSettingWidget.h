// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoundSettingWidget.generated.h"

/**
 * 
 */

class USlider;

UCLASS()
class UNREALPROJECT_API USoundSettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_BGMVolume;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_SFXVolume;

	// 오디오 시스템 조작용 변수들
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundMix* MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundClass* MasterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundClass* BGMClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundClass* SFXClass;

private:
	// 넘겨 받은 세이브 데이터 기억할 포인터
	UPROPERTY()
	class USystemSaveGame* SaveData;

	// 각 슬라이더별 전용 델리게이트 함수
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnBGMVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	// 사운드 적용 및 저장 통합 함수
	void ApplyAndSaveVolume(USoundClass* InClass, float VolumeValue);

public:
	void InitSoundSettings(class USystemSaveGame* InSettings);
};
