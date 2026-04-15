// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "../UnrealProject.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "../Character/UnrealProjectPlayerController.h"
#include "../Interface/HitInterface.h"
#include "../Interface/Interactable.h"

AMeleeWeapon::AMeleeWeapon()
{
	CurrentWeaponStat.WeaponType = EWeaponSlot::Melee;
}

//void AMeleeWeapon::TickAttack()
//{
//	// 현재 소켓 위치 가져오기
//	FVector CurrentTipLocation = WeaponMesh->GetSocketLocation(TEXT("End"));
//
//	// 충돌 검사
//	TArray<FHitResult> HitResults;
//	FCollisionQueryParams QueryParams;
//	QueryParams.AddIgnoredActor(this);
//	QueryParams.AddIgnoredActor(GetOwner());
//
//	bool bHit = GetWorld()->SweepMultiByChannel(
//		HitResults,
//		LastTipLocation, // 이전 프레임
//		CurrentTipLocation, // 현재 프레임
//		FQuat::Identity,
//		ECC_PlayerProjectile,
//		FCollisionShape::MakeSphere(TraceRadius),
//		QueryParams
//	);
//
//	// 디버그
//	DrawDebugSphere(GetWorld(), CurrentTipLocation, TraceRadius, 12, FColor::Red, false, 1.0f);
//
//	// 피격 처리
//	if (bHit) {
//		for (const FHitResult& Hit : HitResults) {
//			AActor* HitActor = Hit.GetActor();
//
//			// 이미 때린 적이면 패스
//			if (!HitActor || IgnoreActors.Contains(HitActor)) continue;
//		
//			// 데미지 적용
//			UGameplayStatics::ApplyDamage(
//				HitActor,
//				Damage,
//				GetInstigatorController(),
//				this,
//				UDamageType::StaticClass()
//			);
//
//			// 목록에 추가
//			IgnoreActors.Add(HitActor);
//
//			/* 타격 이펙트 / 사운드 재생 */
//			
//		}
//	}
//
//	// 현재 위치를 이전 위치로 업데이트
//	LastTipLocation = CurrentTipLocation;
//}

bool AMeleeWeapon::CanAttack()
{
	bool bBaseCan = Super::CanAttack();
	if (!bBaseCan) return false;

	return true;
}

void AMeleeWeapon::ExecuteFire()
{
	Super::ExecuteFire();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->GetController()) return;

	AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(OwnerPawn->GetController());

	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerPawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// 카메라 위치에서 정면으로 AttackRange만큼
	/*FVector Start = ViewLocation;
	FVector End = Start + (ViewRotation.Vector() * AttackRange);*/

	FVector HitLocation = ViewLocation + (ViewRotation.Vector() * (CurrentMeleeStat.AttackRange * 0.5f));

	// 충돌 검사
	TArray<FHitResult> HitResults;
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		HitLocation,
		FQuat::Identity,
		ECC_PlayerProjectile,
		FCollisionShape::MakeSphere(CurrentMeleeStat.TraceRadius),
		QueryParams
	);

	DrawDebugSphere(GetWorld(), HitLocation, CurrentMeleeStat.TraceRadius, 12, FColor::Red, false, 2.0f);

	if (bHit)
	{
		TArray<AActor*> HitActorsThisSwing;

		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();

			// 이미 데미지를 준 적이거나, 적이 아니면 패스
			if (!HitActor || HitActorsThisSwing.Contains(HitActor)) continue;

			FVector SwingDirection;

			if (CurrentMeleeStat.MeleeType == EMeleeType::Thrust)
			{
				SwingDirection = ViewRotation.Vector();
			}
			//플레이어를 중심으로 방사형(부채꼴)
			else if (CurrentMeleeStat.MeleeType == EMeleeType::Sweep)
			{
				SwingDirection = (HitActor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
				SwingDirection.Z = 0.0f;
			}

			// Overlap은 HitResult를 안주기에 Fake로 하나 생성
			FVector HitNormal = -SwingDirection;
			FHitResult FakeHitResult(HitActor, Overlap.GetComponent(), HitActor->GetActorLocation(), HitNormal);

			UGameplayStatics::ApplyPointDamage(
				HitActor,
				CurrentMeleeStat.Damage,
				SwingDirection,
				FakeHitResult,
				OwnerPawn->GetController(),
				this,
				UDamageType::StaticClass()
			);

			HitActorsThisSwing.Add(HitActor);

			if (HitActor->Implements<UHitInterface>())
			{
				IHitInterface::Execute_GetHit(HitActor, HitActor->GetActorLocation());

				if (HitActor->Implements<UInteractable>())
				{
					FText FeedbackText = IInteractable::Execute_GetFeedbackText(HitActor);

					if (!FeedbackText.IsEmpty() && PC)
					{
						PC->ShowFeedback(FeedbackText, IInteractable::Execute_GetFeedbackType(HitActor));
					}
				}
			}
		}
	}
}

void AMeleeWeapon::EndAttack()
{
	// IgnoreActors.Empty();
}

void AMeleeWeapon::InitWeaponData()
{
	if (!WeaponDataHandle.IsNull())
	{
		FMeleeWeaponStatRow* RowData = WeaponDataHandle.GetRow<FMeleeWeaponStatRow>(TEXT("MeleeWeaponDataLookup"));

		if (RowData)
		{
			CurrentMeleeStat = *RowData;
			CurrentWeaponStat = *RowData;
		}
	}
}
