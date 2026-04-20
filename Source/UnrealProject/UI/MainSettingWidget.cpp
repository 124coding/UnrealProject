// Fill out your copyright notice in the Description page of Project Settings.


#include "MainSettingWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "SoundSettingWidget.h"
#include "VideoSettingWidget.h"

#include "../SystemSaveGame.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

void UMainSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 세이브 데이터를 불러오거나 새로 생성해서 CurrentSettings에 채움
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SystemSettings"), 0))
	{
		CurrentSettings = Cast<USystemSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SystemSettings"), 0));
	}
	else {
		CurrentSettings = Cast<USystemSaveGame>(UGameplayStatics::CreateSaveGameObject(USystemSaveGame::StaticClass()));
	}

	if (WBP_VideoSettings) {
		WBP_VideoSettings->InitVideoSettings();
	}

	if (WBP_SoundSettings)
	{
		WBP_SoundSettings->InitSoundSettings(CurrentSettings);
	}

	// 버튼에 클릭 이벤트 묶어주기
	if (Btn_Video) Btn_Video->OnClicked.AddDynamic(this, &UMainSettingWidget::OnVideoBtnClicked);
	if (Btn_Sound) Btn_Sound->OnClicked.AddDynamic(this, &UMainSettingWidget::OnSoundBtnClicked);
	if (Btn_Gameplay) Btn_Gameplay->OnClicked.AddDynamic(this, &UMainSettingWidget::OnGameplayBtnClicked);

	if (Btn_SaveApply) Btn_SaveApply->OnClicked.AddDynamic(this, &UMainSettingWidget::OnSaveBtnClicked);
	if (Btn_Cancel) Btn_Cancel->OnClicked.AddDynamic(this, &UMainSettingWidget::OnCancelBtnClicked);

	// 시작할 때 기본으로 비디오 탭(0번) 띄우기
	if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(0);
}

void UMainSettingWidget::OnVideoBtnClicked()
{
	if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(0);
}

void UMainSettingWidget::OnSoundBtnClicked()
{
	if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(1);
}

void UMainSettingWidget::OnGameplayBtnClicked()
{
	if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(2);
}

void UMainSettingWidget::OnSaveBtnClicked()
{
	if (CurrentSettings)
	{
		UGameplayStatics::SaveGameToSlot(CurrentSettings, TEXT("SystemSettings"), 0);
	}

	if (UGameUserSettings* GraphicsSettings = UGameUserSettings::GetGameUserSettings()) {
		GraphicsSettings->SaveSettings();
		GraphicsSettings->ApplySettings(false);
	}
}

void UMainSettingWidget::OnCancelBtnClicked()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SystemSettings"), 0))
	{
		CurrentSettings = Cast<USystemSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SystemSettings"), 0));

		WBP_SoundSettings->InitSoundSettings(CurrentSettings);
	}

	if (UGameUserSettings* GraphicsSettings = UGameUserSettings::GetGameUserSettings()) {
		GraphicsSettings->LoadSettings(true); // ini 파일 다시 읽어오기
		GraphicsSettings->ApplySettings(false); // 화면 되돌리기
		
		if (WBP_VideoSettings)
		{
			WBP_VideoSettings->InitVideoSettings();
		}
	}

	// 세팅 창 닫기
	RemoveFromParent();
}
