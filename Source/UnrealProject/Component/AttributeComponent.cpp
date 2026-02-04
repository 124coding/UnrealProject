// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// ApplyDamage를 맞을 때마다 RecieveDamage 자동 실행
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UAttributeComponent::RecieveDamage);
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

