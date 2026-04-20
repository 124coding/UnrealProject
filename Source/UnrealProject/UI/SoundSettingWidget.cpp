// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundSettingWidget.h"
#include "../SystemSaveGame.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"

void USoundSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 슬라이더 조작 시 함수 연결 (Bind)
	if (Slider_MasterVolume) Slider_MasterVolume->OnValueChanged.AddDynamic(this, &USoundSettingWidget::OnMasterVolumeChanged);
	if (Slider_BGMVolume) Slider_BGMVolume->OnValueChanged.AddDynamic(this, &USoundSettingWidget::OnBGMVolumeChanged);
	if (Slider_SFXVolume) Slider_SFXVolume->OnValueChanged.AddDynamic(this, &USoundSettingWidget::OnSFXVolumeChanged);
}

void USoundSettingWidget::OnMasterVolumeChanged(float Value)
{
	if (SaveData) SaveData->MasterVolume = Value;
	ApplyAndSaveVolume(MasterClass, Value);
}

void USoundSettingWidget::OnBGMVolumeChanged(float Value)
{
	if (SaveData) SaveData->BGMVolume = Value;
	ApplyAndSaveVolume(BGMClass, Value);
}

void USoundSettingWidget::OnSFXVolumeChanged(float Value)
{
	if (SaveData) SaveData->SFXVolume = Value;
	ApplyAndSaveVolume(SFXClass, Value);
}

void USoundSettingWidget::ApplyAndSaveVolume(USoundClass* InClass, float VolumeValue)
{
	float SafeVolume = FMath::Max(VolumeValue, 0.0001f);

	if (MasterSoundMix && InClass) {
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, InClass, SafeVolume, 1.f, 0.f, true);
	}
}

void USoundSettingWidget::InitSoundSettings(USystemSaveGame* InSettings)
{
	SaveData = InSettings;

	if (SaveData)
	{
		// 슬라이더 손잡이 위치를 저장된 데이터 위치로 갱신
		if (Slider_MasterVolume) Slider_MasterVolume->SetValue(SaveData->MasterVolume);
		if (Slider_BGMVolume) Slider_BGMVolume->SetValue(SaveData->BGMVolume);
		if (Slider_SFXVolume) Slider_SFXVolume->SetValue(SaveData->SFXVolume);

		if (MasterSoundMix)
		{
			UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
		}

		// 초기 볼륨 엔진에 적용
		ApplyAndSaveVolume(MasterClass, SaveData->MasterVolume);
		ApplyAndSaveVolume(BGMClass, SaveData->BGMVolume);
		ApplyAndSaveVolume(SFXClass, SaveData->SFXVolume);
	}
}
