// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyBindEntryWidget.generated.h"

/**
 * 
 */
// 부모에게 바뀐 액션을 알릴 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKeyRebound, class UInputAction*, TargetAction, FName, MappingName, FKey, OldKey, FKey, NewKey);

UCLASS()
class UNREALPROJECT_API UKeyBindEntryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// 액션 이름 표시
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_ActionName;

	// 키 입력 캡처 전용 위젯
	UPROPERTY(meta = (BindWidget))
	class UInputKeySelector* KeySelector;

private:
	UFUNCTION()
	void OnKeySelected(FInputChord SelectedKey);

	UPROPERTY()
	class UInputAction* MyAction;

	UPROPERTY()
	FName MyMappingName;

	UPROPERTY()
	FKey MyOldKey;

public:
	// 액션 할당 함수
	void InitKeyBindEntry(class UInputAction* InAction, FName InMappingName, FText InDisplayName, FKey InCurrentKey);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnKeyRebound OnKeyRebound;
};
