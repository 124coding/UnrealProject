// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectorDebugWidget.h"
#include "Components/TextBlock.h"
#include "../DirectorDataSubsystem.h" 
#include "../UnrealProjectGameMode.h" 
#include "Kismet/GameplayStatics.h"

void UDirectorDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 디렉터 서브시스템에서 스트레스 수치 가져오기
	if (UDirectorDataSubsystem* DataSubsystem = GetWorld()->GetSubsystem<UDirectorDataSubsystem>()) {
		if (Text_StressLevel)
		{
			FString StressString = FString::Printf(TEXT("Stress: %.1f / 100"), DataSubsystem->CurrentPlayerStress);
			Text_StressLevel->SetText(FText::FromString(StressString));
		}
	}

	// 현재 페이즈와 남은 적 수
	if (AUnrealProjectGameMode* GM = Cast<AUnrealProjectGameMode>(UGameplayStatics::GetGameMode(this))) {
		if (Text_CurrentPhase) {
			FString PhaseName = TEXT("Unknown");

			switch (GM->CurrentPhase)
			{
			case EDirectorPhase::Relax:
				PhaseName = TEXT("RELAX");
				break;

			case EDirectorPhase::BuildUp: 
				PhaseName = TEXT("BUILD UP");
				break;

			case EDirectorPhase::Peak:    
				PhaseName = TEXT("PEAK");
				break;

			case EDirectorPhase::FadeOut: 
				PhaseName = TEXT("FADE OUT"); 
				break;
			}

			Text_CurrentPhase->SetText(FText::FromString(FString::Printf(TEXT("Phase: %s"), *PhaseName)));

			if (Text_AliveEnemies) {
				FString EnemyString = FString::Printf(TEXT("Alive Enemies: %d"), GM->GetAliveEnemyCount());
				Text_AliveEnemies->SetText(FText::FromString(EnemyString));
			}
		}

		if (Text_CurrentTokens) {
			FString TokensString = FString::Printf(TEXT("Current Tokens: %d"), GM->GetCurrentTokens());
			Text_CurrentTokens->SetText(FText::FromString(TokensString));
		}
	}
}
