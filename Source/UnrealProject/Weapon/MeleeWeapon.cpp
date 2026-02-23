// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "../UnrealProject.h"
#include "Kismet/GameplayStatics.h"

AMeleeWeapon::AMeleeWeapon()
{
	WeaponType = EWeaponSlot::Melee;
}

void AMeleeWeapon::TickAttack()
{
	// 현재 소켓 위치 가져오기
	FVector CurrentTipLocation = WeaponMesh->GetSocketLocation(TEXT("End"));

	// 충돌 검사
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		LastTipLocation, // 이전 프레임
		CurrentTipLocation, // 현재 프레임
		FQuat::Identity,
		ECC_PlayerProjectile,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams
	);

	// 디버그
	DrawDebugSphere(GetWorld(), CurrentTipLocation, TraceRadius, 12, FColor::Red, false, 1.0f);

	// 피격 처리
	if (bHit) {
		for (const FHitResult& Hit : HitResults) {
			AActor* HitActor = Hit.GetActor();

			// 이미 때린 적이면 패스
			if (!HitActor || IgnoreActors.Contains(HitActor)) continue;
		
			// 데미지 적용
			UGameplayStatics::ApplyDamage(
				HitActor,
				Damage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);

			// 목록에 추가
			IgnoreActors.Add(HitActor);

			/* 타격 이펙트 / 사운드 재생 */
			
		}
	}

	// 현재 위치를 이전 위치로 업데이트
	LastTipLocation = CurrentTipLocation;
}

bool AMeleeWeapon::CanAttack()
{
	bool bBaseCan = Super::CanAttack();
	if (!bBaseCan) return false;

	return true;
}

void AMeleeWeapon::OnAttack()
{
	// 중복 타격 목록 초기화
	IgnoreActors.Empty();

	// 시작 시점의 소켓 위치 저장
	LastTipLocation = WeaponMesh->GetSocketLocation(TEXT("End"));
}

void AMeleeWeapon::EndAttack()
{
	IgnoreActors.Empty();
}
