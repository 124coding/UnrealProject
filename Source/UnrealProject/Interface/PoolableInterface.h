

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PoolableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNREALPROJECT_API IPoolableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 현재 풀에서 활성화되어 사용 중인지 확인하는 함수
	virtual bool IsActiveInPool() const = 0;

	// 풀 활성화/비활성화 상태를 세팅하는 함수
	virtual void SetActiveInPool(bool bActive) = 0;

	// 풀에서 꺼내질 때 (SetActive) - 변수 초기화, 물리 켜기
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnPoolSpawned();

	// 풀로 돌아갈 때 (SetInactive) - 물리 끄기, 숨기기
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnPoolReturned();
	
	// Pool에 넣는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void SetOwningPool(UObjectPoolComponent* NewPool);
};
