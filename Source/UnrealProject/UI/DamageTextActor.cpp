// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "../Component/ObjectPoolComponent.h"

// Sets default values
ADamageTextActor::ADamageTextActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	DamageWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	DamageWidget->SetupAttachment(Root);

	// 위젯은 항상 플레이어를 보게 ScreenMode
	DamageWidget->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidget->SetDrawSize(FVector2D(200.0f, 50.0f));

}

void ADamageTextActor::ShowDamage(const FVector& Location, float DamageAmount)
{
	SetActorLocation(Location);

	// 블루프린트 이벤트 호출
	SetupDamageText(DamageAmount);

	// 1.5초 뒤에 풀로 반납
	GetWorld()->GetTimerManager().SetTimer(
		ReturnTimerHandle,
		this,
		&ADamageTextActor::SelfReturnToPool,
		1.5f,
		false
	);
}

void ADamageTextActor::OnPoolSpawned_Implementation()
{
	SetActorHiddenInGame(false);
}

void ADamageTextActor::OnPoolReturned_Implementation()
{
	SetActorHiddenInGame(true);
	GetWorld()->GetTimerManager().ClearTimer(ReturnTimerHandle);
}

void ADamageTextActor::SetOwningPool_Implementation(UObjectPoolComponent* NewPool)
{
	this->OwningPoolComponent = NewPool;
}

// Called when the game starts or when spawned
void ADamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorHiddenInGame(true);
}

void ADamageTextActor::SelfReturnToPool()
{
	if (OwningPoolComponent)
	{
		OwningPoolComponent->ReturnToPool(this);
	}
}

