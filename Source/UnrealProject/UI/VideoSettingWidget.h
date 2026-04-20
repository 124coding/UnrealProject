// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VideoSettingWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API UVideoSettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:

	// 해상도와 창 모드 선택 콤보박스
	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* ComboBox_Resolution;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* ComboBox_WindowMode;

private:
	// 콤보박스 값이 바뀔 때 실행될 이벤트
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// 엔진의 그래픽 설정 조작 포인터
	UPROPERTY()
	class UGameUserSettings* UserSettings;

public:
	// 메인 위젯이 호출할 초기화 함수
	void InitVideoSettings();
};
