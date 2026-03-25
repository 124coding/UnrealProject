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

			// 스폰 위치에 장애물이 있는지 확인
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			bool bHit = GetWorld()->OverlapBlockingTestByChannel(
				CandidateLoc,
				FQuat::Identity,
				ECollisionChannel::ECC_Visibility,
				FCollisionShape::MakeSphere(50.0f), /* 현재 하드 코딩으로 50.0f 값이 들어가 있기에 추후 적의 크기에 맞춰서 수정 필요 */
				Params
			);

			// 충돌이 없으면 플레이어 시야 검사
			if (!bHit) {
				APlayerController* PC = GetWorld()->GetFirstPlayerController();

				if (PC && PC->GetPawn()) {
					FVector CameraLoc;
					FRotator CameraRot;
					PC->GetPlayerViewPoint(CameraLoc, CameraRot);

					FHitResult LineHit;
					FCollisionQueryParams LineParams;
					LineParams.AddIgnoredActor(this);
					LineParams.AddIgnoredActor(PC->GetPawn()); // 플레이어는 무시

					// 카메라 위치에서 스폰 지점으로 RayCast
					bool bHitWall = GetWorld()->LineTraceSingleByChannel(
						LineHit,
						CameraLoc,
						CandidateLoc,
						ECC_Visibility,
						LineParams
					);

					if (bHitWall) {
						return CandidateLoc;
					}
					else {
						// 모니터 화면 내에 보이는 위치인지 판별
						FVector2D ScreenPosition;

						// 스폰 좌표를 모니터 화면 좌표로 변환
						bool bIsInFrontOfCamera = PC->ProjectWorldLocationToScreen(CandidateLoc, ScreenPosition);

						if (bIsInFrontOfCamera) {
							int32 ViewportSizeX, ViewportSizeY;
							PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

							// 여유 공간을 줘서 화면 밖 100픽셀 밖에서 스폰 보정
							float Margin = 100.f;

							bool bIsOnScreen = (ScreenPosition.X >= -Margin && ScreenPosition.X <= ViewportSizeX + Margin) &&
											   (ScreenPosition.Y >= -Margin && ScreenPosition.Y <= ViewportSizeY + Margin);

							if (bIsOnScreen) continue;
							else return CandidateLoc;
						}
					}
				}
				else
				{
					// 플레이어를 못 찾았다면 그냥 스폰
					return CandidateLoc;
				}
			}
		}
	}

	return Origin; // 실패 시 중앙
}

