// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvQueryGenerator_DensityDonut.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Density Donut"))
class UNREALPROJECT_API UEnvQueryGenerator_DensityDonut : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_DensityDonut();
	
public:
	// 안쪽 원의 반지름
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue InnerRadius;

	// 바깥쪽 원의 반지름
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue OuterRadius;

	// 생성할 원(Ring)의 개수
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderIntValue NumberOfRings;

	// 가장 안쪽 원의 기본 점 개수 (바깥으로 갈수록 비율에 맞춰 자동 증가)
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderIntValue BasePointsPerRing;

	// 점들을 생성할 중심 기준점 (Context)
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<class UEnvQueryContext> SearchCenter;

	// 아이템 생성 함수
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	// 에디터 UI에 보여질 텍스트
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
