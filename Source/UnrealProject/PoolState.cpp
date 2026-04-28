// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolState.h"
#include "Component/ObjectPoolComponent.h"

APoolState::APoolState()
{
	DamageTextPool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("DamageTextPool"));
}

void APoolState::BeginPlay()
{
	Super::BeginPlay();

	if (DamageTextPool && DamageTextClass) {
		DamageTextPool->InitializePool(DamageTextClass, 30);
	}
}
