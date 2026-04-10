// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DirectorControlVolume.generated.h"

UENUM(BlueprintType)
enum class EZoneType : uint8
{
	None,
	SafeZone,    // 시작 지점, 중간 휴식방
	ChokePoint,  // 좁은 복도 (웨이브 억제)
	Arena
};

UCLASS()
class UNREALPROJECT_API ADirectorControlVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ADirectorControlVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Director")
	EZoneType ZoneType = EZoneType::None;

	UFUNCTION()
	void OnZoneEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnZoneExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
