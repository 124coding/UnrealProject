// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPoolComponent.h"
#include "../PoolableInterface.h"

// Sets default values for this component's properties
UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::InitializePool(TSubclassOf<AActor> ClassToPool, int32 Count)
{
	PooledClass = ClassToPool;

	for (int32 i = 0; i < Count; i++) {
		CreateNewObject();
	}
}

AActor* UObjectPoolComponent::SpawnFromPool(FVector SpawnLocation, FRotator SpawnRotation)
{
	AActor* PooledActor = nullptr;

	// 풀에 남은게 있는지 확인 후 없다면 동적 확장
	if (PoolQueue.Num() > 0) {
		PooledActor = PoolQueue.Pop();
	}
	else {
		PooledActor = CreateNewObject();
		
		// 방금 만든건 PoolQueue에 들어갔으니 다시 pop
		if (PoolQueue.Num() > 0) PoolQueue.Pop();
	}

	if (PooledActor) {
		PooledActor->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);

		if (PooledActor->Implements<UPoolableInterface>()) {
			IPoolableInterface::Execute_OnPoolSpawned(PooledActor);
		}
		else {
			// 인터페이스 없으면 수동 설정
			PooledActor->SetActorHiddenInGame(false);
			PooledActor->SetActorEnableCollision(true);
			PooledActor->SetActorTickEnabled(true);
		}
	}

	return nullptr;
}

void UObjectPoolComponent::ReturnToPool(AActor* ActorToReturn)
{
	if (!ActorToReturn) return;

	if (ActorToReturn->Implements<UPoolableInterface>()) {
		IPoolableInterface::Execute_OnPoolReturned(ActorToReturn);
	}
	else {
		ActorToReturn->SetActorHiddenInGame(true);
		ActorToReturn->SetActorEnableCollision(false);
		ActorToReturn->SetActorTickEnabled(false);
	}

	PoolQueue.Add(ActorToReturn);
}

AActor* UObjectPoolComponent::CreateNewObject() {
	if (!PooledClass) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 생성하자마자 비활성화
	AActor* NewActor = World->SpawnActor<AActor>(PooledClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (NewActor) {
		// 인터페이스가 있다면 비활성화 로직 실행
		if (NewActor->Implements<UPoolableInterface>()) {
			IPoolableInterface::Execute_OnPoolReturned(NewActor);
		}
		else {
			// 인터페이스 안 쓰면 기본적으로 숨김
			NewActor->SetActorHiddenInGame(true);
			NewActor->SetActorEnableCollision(false);
			NewActor->SetActorTickEnabled(false);
		}

		PoolQueue.Add(NewActor);
	}

	return NewActor;
}
