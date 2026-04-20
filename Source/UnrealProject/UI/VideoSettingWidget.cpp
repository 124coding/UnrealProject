// Fill out your copyright notice in the Description page of Project Settings.


#include "VideoSettingWidget.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"

void UVideoSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 엔진의 기본 세팅 객체 가져옴
	UserSettings = UGameUserSettings::GetGameUserSettings();

	// 콤보박스 이벤트
	if (ComboBox_Resolution) {
		ComboBox_Resolution->OnSelectionChanged.AddDynamic(this, &UVideoSettingWidget::OnResolutionChanged);
	}
	if (ComboBox_WindowMode) {
		ComboBox_WindowMode->OnSelectionChanged.AddDynamic(this, &UVideoSettingWidget::OnWindowModeChanged);
	}
}

void UVideoSettingWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!UserSettings || SelectionType == ESelectInfo::Direct) return;

	// 문자열을 X와 Y로 쪼개기
	FString LeftStr, RightStr;
	SelectedItem.Split(TEXT("x"), &LeftStr, &RightStr);

	FIntPoint NewRes(FCString::Atoi(*LeftStr), FCString::Atoi(*RightStr));

	// 메모리에 새 해상도 적용
	UserSettings->SetScreenResolution(NewRes);

	// 화면 즉시 갱신(미리보기)
	UserSettings->ApplyResolutionSettings(false);
}

void UVideoSettingWidget::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!UserSettings || SelectionType == ESelectInfo::Direct) return;

	int32 SelectedIndex = ComboBox_WindowMode->FindOptionIndex(SelectedItem);

	EWindowMode::Type NewMode = EWindowMode::Windowed;

	switch (SelectedIndex)
	{
		case 0: NewMode = EWindowMode::Fullscreen; break;        
		case 1: NewMode = EWindowMode::WindowedFullscreen; break;
		case 2: NewMode = EWindowMode::Windowed; break;          
	}

	UserSettings->SetFullscreenMode(NewMode);
	UserSettings->ApplyResolutionSettings(false);
}

void UVideoSettingWidget::InitVideoSettings()
{
	if (!UserSettings) return;

	// 초기 세팅
	FIntPoint CurrentRes = UserSettings->GetScreenResolution();
	FString ResString = FString::Printf(TEXT("%dx%d"), CurrentRes.X, CurrentRes.Y);
	ComboBox_Resolution->SetSelectedOption(ResString);

	// 창 모드 표시
	int32 WindowMode = (int32)UserSettings->GetFullscreenMode();
	if (WindowMode == 0) ComboBox_WindowMode->SetSelectedOption(TEXT("전체화면"));
	else if (WindowMode == 1) ComboBox_WindowMode->SetSelectedOption(TEXT("전체 창모드"));
	else ComboBox_WindowMode->SetSelectedOption(TEXT("창모드"));
}
