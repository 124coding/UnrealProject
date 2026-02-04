// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectPoolComponent();
	

public:	
	// 풀 초기화 (어떤 클래스를 몇 개 만들건지)
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void InitializePool(TSubclassOf<AActor> ClassToPool, int32 Count);

	// 풀에서 하나만 가져오기
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* SpawnFromPool(FVector SpawnLocation, FRotator SpawnRotation);

	// 풀로 돌려보내기
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnToPool(AActor* ActorToReturn);

protected:
	UPROPERTY()
	TSubclassOf<AActor> PooledClass;

	// 대기 중인 오브젝트들
	UPROPERTY()
	TArray<AActor*> PoolQueue;

	// 풀 확장 (부족할 시 추가 생성)
	AActor* CreateNewObject();
};
