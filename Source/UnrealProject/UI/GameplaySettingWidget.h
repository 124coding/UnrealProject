// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplaySettingWidget.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FKeyBindData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	class UInputAction* Action;

	UPROPERTY(EditDefaultsOnly)
	FName MappingName;

	UPROPERTY(EditDefaultsOnly)
	FText DisplayName; // UI에 보여줄 글씨
};

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

	// 에디터에서 유저에게 보여줄 액션들
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FKeyBindData> RebindableActions;

	// 자식 위젯의 블루프린트 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<class UKeyBindEntryWidget> KeyBindEntryClass;

	// 자식들을 담을 ScrollBox
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox_KeyBinds;

private:
	UFUNCTION()
	void OnMouseSensChanged(float Value);

	UPROPERTY()
	class USystemSaveGame* SaveData;

private:
	// 키 리바인딩 수행 함수
	UFUNCTION()
	void RebindActionKey(class UInputAction* TargetAction, FName MappingName, FKey OldKey, FKey NewKey);

	FKey FindKeyForAction(class UInputMappingContext* IMC, class UInputAction* TargetAction, FName MappingName);

	// 에디터에서 설정할 매핑 컨텍스트와 인풋 액션들
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* DefaultIMC;

public:
	// 메인에서 부를 초기화 함수
	void InitGameplaySettings(class USystemSaveGame* InSettings);

	// 메인에서 부를 Apply 함수
	void ApplyKeyBindingsToIMC();
};
