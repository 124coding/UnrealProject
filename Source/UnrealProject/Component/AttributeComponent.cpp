// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"
#include "GameFramework/Actor.h"
#include "../SurvivalGameInstance.h"
#include "../Projectile/UnrealProjectProjectile.h"
#include "../Weapon/BaseWeapon.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeStats();

	// ApplyDamage를 맞을 때마다 RecieveDamage 자동 실행
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UAttributeComponent::RecieveDamage);
	GetOwner()->OnTakeRadialDamage.AddDynamic(this, &UAttributeComponent::ReceiveRadialDamage);
	GetOwner()->OnTakePointDamage.AddDynamic(this, &UAttributeComponent::ReceivePointDamage);
	// ...
	
}


// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAttributeComponent::InitializeStats()
{
	CurrentHealth = MaxHealth;

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UAttributeComponent::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || CurrentHealth <= 0.0f) return;

	// 체력이 0 ~ MaxHealth 사이일 수 있도록
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	// 체력이 변경되었음을 알림
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("Health Changed: %f"), CurrentHealth);
	
	// 체력이 0이 되면 죽었다고 알림
	if (CurrentHealth <= 0.0f)
	{
		AActor* KillerActor = nullptr;

		if (InstigatedBy)
		{
			KillerActor = InstigatedBy->GetPawn();
		}
		else if (DamageCauser)
		{
			KillerActor = DamageCauser;
		}

		OnDeath.Broadcast(GetOwner(), KillerActor);
	}
}

void UAttributeComponent::ReceiveRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	if (CurrentHealth <= 0.f) return;

	float KnockbackForce = 0.f;

	if (AUnrealProjectProjectile* Projectile = Cast<AUnrealProjectProjectile>(DamageCauser)) {
		KnockbackForce = Projectile->GetKnockbackPower();

		// 폭발 중심점으로부터 적까지의 거리
		float Distance = FVector::Dist(this->GetOwner()->GetActorLocation(), Origin);

		// 투사체가 넘겨준 폭발 반경 정보
		float InnerRadius = Projectile->GetInnerRadius();
		float OuterRadius = Projectile->GetOuterRadius();

		// 거리에 따른 넉백 감소 비율 계산
		FVector2D InputRange(InnerRadius, OuterRadius); // 거리 범위
		FVector2D OutputRange(1.0f, 0.4f); // 배수 범위

		// 거리에 따른 자연스러운 보간
		float FalloffMultiplier = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, Distance);

		KnockbackForce *= FalloffMultiplier;
	}

	if (KnockbackForce > 0.0f)
	{
		FVector PushDirection = (GetOwner()->GetActorLocation() - Origin).GetSafeNormal2D();

		// 넉백 이벤트
		OnKnockback.Broadcast(PushDirection, KnockbackForce);
	}
}

void UAttributeComponent::ReceivePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (CurrentHealth <= 0.f) return;

	float KnockbackForce = 0.f;

	if (ABaseWeapon* Weapon = Cast<ABaseWeapon>(DamageCauser))
	{
		KnockbackForce = Weapon->GetKnockbackPower();
	}

	if (KnockbackForce > 0.0f) {
		FVector PushDirection = ShotDirection.GetSafeNormal();
		PushDirection.Z = 0.2f;
		PushDirection.Normalize();
	
		OnKnockback.Broadcast(PushDirection, KnockbackForce);
	}
}

void UAttributeComponent::MaxHealthUp(float Amount)
{
	MaxHealth += Amount;

	if (OnHealthChanged.IsBound()) {
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UAttributeComponent::Heal(float Amount)
{
	CurrentHealth += Amount;

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

	if (OnHealthChanged.IsBound()) {
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UAttributeComponent::SaveDataToGI(USurvivalGameInstance* GI)
{
	if (!GI) return;

	GI->PlayerAttributeData.MaxHealth = MaxHealth;
	GI->PlayerAttributeData.CurrentHealth = CurrentHealth;
}

void UAttributeComponent::LoadDataFromGI(USurvivalGameInstance* GI)
{
	if (!GI || !GI->bIsSaveDataValid) return;
	MaxHealth = GI->PlayerAttributeData.MaxHealth;
	CurrentHealth = GI->PlayerAttributeData.CurrentHealth;
}

