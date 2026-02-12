// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "UnrealProject.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	RootComponent = SpawningBox;

	// 박스 크기 초기값
	SpawningBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));

	// 게임에서 박스는 안보이게 (디버그용 false)
	SpawningBox->SetHiddenInGame(false);

	SpawningBox->SetCollisionProfileName(TEXT("OverlapAll"));
}

FVector ASpawnVolume::GetRandomPointInVolume()
{
	FVector Origin = SpawningBox->GetComponentLocation();
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	const int32 MaxAttempts = 10; // 최대 10번 시도

	for (int32 i = 0; i < MaxAttempts; i++) {
		// 박스 안의 랜덤 위치
		FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

		// NavMesh 투영
		FNavLocation NavLoc;
		const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		if (NavSystem && NavSystem->ProjectPointToNavigation(RandomPoint, NavLoc)) {
			FVector CandidateLoc = NavLoc.Location;
			CandidateLoc.Z += 100.0f;

			// 스폰 위치에 Pawn이 있는지 확인
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			bool bHit = GetWorld()->OverlapBlockingTestByChannel(
				CandidateLoc,
				FQuat::Identity,
				ECollisionChannel::ECC_Enemy,
				FCollisionShape::MakeSphere(50.0f), /* 현재 하드 코딩으로 50.0f 값이 들어가 있기에 추후 적의 크기에 맞춰서 수정 필요 */
				Params
			);

			// 충돌이 없으면 이 좌표 당첨;
			if (!bHit) {
				return CandidateLoc;
			}
		}
	}

	return Origin; // 실패 시 중앙
}

