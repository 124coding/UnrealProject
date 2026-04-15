// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableBaseActor.h"

#include "Interface/HitInterface.h" 

#include "PanicEventActor.generated.h"

UCLASS()
class UNREALPROJECT_API APanicEventActor : public AInteractableBaseActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APanicEventActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* OverlapVolume;

	// 한 번 발동되면 다시 발동되지 않도록 막는 플래그
	bool bHasBeenTriggered = false;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bInteractTriggered = false;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bHitTriggered = false;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bOverlapTriggered = false;

	// 상호작용(Interact)으로 작동시킬 때
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	// 총으로 쏴서(Hit) 작동시킬 때
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	UFUNCTION()
	void OnPanicOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 블루프린트에서 사운드/이펙트를 재생하기 위한 이벤트
	/*UFUNCTION(BlueprintImplementableEvent, Category = "Panic Event")
	void PlayPanicEffects();*/

private:
	// 실제 디렉터에게 스트레스 폭발을 명령하는 핵심 내부 함수
	void ExecutePanicEvent();
};
