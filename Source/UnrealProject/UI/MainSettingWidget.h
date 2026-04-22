// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainSettingWidget.generated.h"

/**
 * 
 */
class UButton;

UCLASS()
class UNREALPROJECT_API UMainSettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Video;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Sound;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Gameplay;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Save;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Apply;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Cancel;

	UPROPERTY(meta = (BindWidget))
	class UVideoSettingWidget* WBP_VideoSettings;

	UPROPERTY(meta = (BindWidget))
	class USoundSettingWidget* WBP_SoundSettings;

	UPROPERTY(meta = (BindWidget))
	class UGameplaySettingWidget* WBP_GameplaySettings;

	// 화면을 교체할 위젯 스위처
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* PanelSwitcher;

private:

	// 버튼 클릭 이벤트 함수
	UFUNCTION() 
	void OnVideoBtnClicked();

	UFUNCTION() 
	void OnSoundBtnClicked();
	
	UFUNCTION() 
	void OnGameplayBtnClicked();

	UFUNCTION()
	void OnSaveBtnClicked();

	UFUNCTION()
	void OnApplyBtnClicked();

	UFUNCTION()
	void OnCancelBtnClicked();

public:
	UPROPERTY()
	class USystemSaveGame* CurrentSettings;
};
