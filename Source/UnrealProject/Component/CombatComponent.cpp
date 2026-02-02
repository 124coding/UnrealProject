// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "../Character/UnrealProjectCharacter.h"
#include "../Weapon/RangedWeapon.h"
#include "GameFramework/Character.h"
#include "EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 테스트용 기본 무기 생성
	if (DefaultWeaponClass) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();

		ABaseWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
			DefaultWeaponClass,
			GetOwner()->GetActorLocation(),
			GetOwner()->GetActorRotation(),
			SpawnParams
		);

		if (SpawnedWeapon)
		{
			PickupWeapon(SpawnedWeapon);
		}
	}

	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::PickupWeapon(ABaseWeapon* NewWeapon)
{
	if (NewWeapon == nullptr) return;

	EWeaponSlot Slot = NewWeapon->WeaponType;

	// 무기의 타입에 맞는 슬롯이 비었는지 확인하고 비어 있지 않으면 해당 슬롯에 있는 무기를 떨어뜨림
	if (CarriedWeapons.Contains(Slot)) {
		ABaseWeapon* OldWeapon = CarriedWeapons[Slot];

		if (OldWeapon) {
			OldWeapon->Destroy(); // 일단 제거
			// DropWeapon(OldWeapon);	 /* 떨어뜨리는 로직 필요 */
		}
	}

	CarriedWeapons.Add(Slot, NewWeapon);
	NewWeapon->SetOwner(GetOwner());

	// 현재 들고 있는 무기가 없으면 바로 장착
	if (CurrentWeapon == nullptr) {
		EquipWeaponBySlot(Slot);
	}
	else {
		// 당장 사용하지 않을 시 숨김
		NewWeapon->SetActorHiddenInGame(true);
		NewWeapon->SetActorEnableCollision(false);
	}
}

void UCombatComponent::EquipWeaponBySlot(EWeaponSlot SlotToEquip)
{
	if (!CarriedWeapons.Contains(SlotToEquip)) return;

	ABaseWeapon* WeaponToEquip = CarriedWeapons[SlotToEquip];
	
	// 현재 들고 있는 무기는 집어넣기
	if (CurrentWeapon) CurrentWeapon->SetActorHiddenInGame(true);

	AUnrealProjectCharacter* Owner = Cast<AUnrealProjectCharacter>(GetOwner());
	if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 이전 무기의 컨텍스트 제거
			if (CurrentWeapon && CurrentWeapon->WeaponMappingContext)
			{
				Subsystem->RemoveMappingContext(CurrentWeapon->WeaponMappingContext);
			}

			// 새 무기의 컨텍스트 추가
			if (WeaponToEquip && WeaponToEquip->WeaponMappingContext)
			{
				// Priority를 높게 줘서 기본 키 설정을 덮어쓰게 할 수도 있음
				Subsystem->AddMappingContext(WeaponToEquip->WeaponMappingContext, 1);
			}
		}
	}

	CurrentWeapon = WeaponToEquip;
	CurrentWeapon->SetActorHiddenInGame(false);

	AttachWeaponToHand(CurrentWeapon);
}

void UCombatComponent::AttachWeaponToHand(ABaseWeapon* Weapon)
{
	AUnrealProjectCharacter* Owner = Cast<AUnrealProjectCharacter>(GetOwner());
	if (Owner && Weapon) {
		Weapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		Weapon->AttachToComponent(
			Owner->GetMesh1P(),
			AttachmentRules,
			TEXT("GripPoint") /* 캐릭터 메쉬의 손 소켓 이름 */
		);
	}
}

void UCombatComponent::Attack() {
	if (CurrentWeapon)
	{
		CurrentWeapon->Attack();
	}
}

void UCombatComponent::StopAttack()
{
	// 현재 들고 있는 무기가 원거리 무기라면 리셋 신호 보냄
	if (ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon))
	{
		RangedWeapon->StopAttack();
	}
}

void UCombatComponent::Reload() {
	if (CurrentWeapon == nullptr) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon);

	if (RangedWeapon) RangedWeapon->Reload();
	else UE_LOG(LogTemp, Warning, TEXT("This weapon cant Reload"));
}
