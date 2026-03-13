// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryGenerator_DensityDonut.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UEnvQueryGenerator_DensityDonut::UEnvQueryGenerator_DensityDonut()
{
	ItemType = UEnvQueryItemType_Point::StaticClass();

	SearchCenter = UEnvQueryContext_Querier::StaticClass();

	// 에디터 기본값 세팅
	InnerRadius.DefaultValue = 100.0f;
	OuterRadius.DefaultValue = 600.0f;
	NumberOfRings.DefaultValue = 5;
	BasePointsPerRing.DefaultValue = 6;
}

void UEnvQueryGenerator_DensityDonut::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr) return;

	// 에디터 입력 변수값 바인딩
	InnerRadius.BindData(QueryOwner, QueryInstance.QueryID);
	OuterRadius.BindData(QueryOwner, QueryInstance.QueryID);
	NumberOfRings.BindData(QueryOwner, QueryInstance.QueryID);
	BasePointsPerRing.BindData(QueryOwner, QueryInstance.QueryID);

	float RadiusMin = InnerRadius.GetValue();
	float RadiusMax = OuterRadius.GetValue();
	int32 Rings = NumberOfRings.GetValue();
	int32 BasePoints = BasePointsPerRing.GetValue();

	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(SearchCenter, ContextLocations);
	TArray<FNavLocation> GeneratedItems;

	if (Rings > 0 && RadiusMax > RadiusMin && ContextLocations.Num() > 0) {
		float RingStep = (RadiusMax - RadiusMin) / (float)Rings;

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++) {
			FVector Center = ContextLocations[ContextIndex];

			for (int32 RingIndex = 0; RingIndex < Rings; RingIndex++) {
				float CurrentRadius = RadiusMin + (RingStep * RingIndex);

				// 바깥쪽 원일수록 면적이 커지는 비율(Ratio) 계산
				float Ratio = CurrentRadius / RadiusMin;

				// 비율을 곱해서 이 Ring에 배치될 진짜 점의 개수 계산
				int32 PointsThisRing = FMath::RoundToInt(BasePoints * Ratio);

				float AngleStep = 360.0f / (float)PointsThisRing;

				for (int32 PointIndex = 0; PointIndex < PointsThisRing; PointIndex++) {
					float Angle = AngleStep * PointIndex;

					// 각도를 벡터 방향으로 변환
					FVector Dir = FRotator(0.0f, Angle, 0.0f).Vector();
					FVector Point = Center + (Dir * CurrentRadius);

					GeneratedItems.Add(FNavLocation(Point));
				}
			}
		}
	}

	// 생성된 점들을 NavMesh 위로 투영(Project)하고 결과를 저장
	ProjectAndFilterNavPoints(GeneratedItems, QueryInstance);
	StoreNavPoints(GeneratedItems, QueryInstance);
}

FText UEnvQueryGenerator_DensityDonut::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("Density Donut (Optimized)"));
}

FText UEnvQueryGenerator_DensityDonut::GetDescriptionDetails() const
{
	return FText::FromString(TEXT("Outer rings generate more points proportionally."));
}
