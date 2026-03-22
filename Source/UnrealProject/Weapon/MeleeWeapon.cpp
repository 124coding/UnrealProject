// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "../UnrealProject.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

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

	// Sweep 방식
	/*bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_PlayerProjectile,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams
	);*/

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		HitLocation,
		FQuat::Identity,
		ECC_PlayerProjectile,
		FCollisionShape::MakeSphere(CurrentMeleeStat.TraceRadius),
		QueryParams
	);

	// Sweep 방식의 Capsule
	/*FVector TraceDirection = (End - Start).GetSafeNormal();

	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceDirection).ToQuat(); 

	DrawDebugCapsule(
		GetWorld(),
		Start + (End - Start) * 0.5f,
		AttackRange * 0.5f,          
		TraceRadius,                 
		CapsuleRot,                  
		FColor::Red,
		false,
		2.0f
	);*/

	DrawDebugSphere(GetWorld(), HitLocation, CurrentMeleeStat.TraceRadius, 12, FColor::Red, false, 2.0f);

	// Sweep 방식
	//if (bHit)
	//{
	//	// 한 번의 휘두르기에 한 놈이 두 번 맞는 걸 방지하기 위한 지역 변수
	//	TSet<AActor*> HitActorsThisSwing;

	//	for (const FHitResult& Hit : HitResults)
	//	{
	//		AActor* HitActor = Hit.GetActor();

	//		// 이미 데미지를 준 적이면 패스
	//		if (!HitActor || HitActorsThisSwing.Contains(HitActor)) continue;

	//		// 데미지 적용
	//		UGameplayStatics::ApplyDamage(
	//			HitActor,
	//			Damage,
	//			OwnerPawn->GetController(),
	//			this,
	//			UDamageType::StaticClass()
	//		);

	//		// 목록에 추가
	//		HitActorsThisSwing.Add(HitActor);
	//	}
	//}

	if (bHit)
	{
		TArray<AActor*> HitActorsThisSwing;

		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();

			// 이미 데미지를 준 적이거나, 적이 아니면 패스
			if (!HitActor || HitActorsThisSwing.Contains(HitActor)) continue;

			UGameplayStatics::ApplyDamage(
				HitActor,
				CurrentMeleeStat.Damage,
				OwnerPawn->GetController(),
				this,
				UDamageType::StaticClass()
			);

			HitActorsThisSwing.Add(HitActor);
		}
	}

	// 중복 타격 목록 초기화
	// IgnoreActors.Empty();

	// 시작 시점의 소켓 위치 저장
	// LastTipLocation = WeaponMesh->GetSocketLocation(TEXT("End"));
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
