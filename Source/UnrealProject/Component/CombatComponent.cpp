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
	/*if (DefaultWeaponClass) {
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
	}*/

	// ...
	
}

void UCombatComponent::DropWeaponSafeLocation(ABaseWeapon* WeaponToDrop)
{
	if (!WeaponToDrop) return;

	AUnrealProjectCharacter* PlayerChar = Cast<AUnrealProjectCharacter>(GetOwner());
	if (!PlayerChar || !PlayerChar->GetController()) return;

	// 카메라 위치와 시선
	FVector Location;
	FRotator Rotation;
	PlayerChar->GetController()->GetPlayerViewPoint(Location, Rotation);

	// 떨어뜨릴 목표 지점 계산
	float DropDistance = 150.0f;
	FVector TraceStart = Location;
	FVector TraceEnd = Location + (Rotation.Vector() * DropDistance);

	// 벽 뚫기 방지
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerChar);
	QueryParams.AddIgnoredActor(WeaponToDrop);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// 최종 위치 결정
	FVector FinalLocation;

	if (bHit) {
		// 벽에 막혔으면 벽보다 살짝 앞으로
		// 벽이랑 너무 가까우면(30cm 이내) 그냥 내 발밑에 떨굼
		if (HitResult.Distance < 30.0f)
		{
			// 그냥 캡슐(발) 위치에서 살짝 앞으로
			FinalLocation = PlayerChar->GetActorLocation() + (PlayerChar->GetActorForwardVector() * 20.0f);
		}
		else
		{
			// 충분히 머니까 벽 앞에서 10cm 띄움
			FinalLocation = HitResult.Location - (Rotation.Vector() * 10.0f);
		}
	}
	else {
		FinalLocation = TraceEnd;
	}

	FRotator DropRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);

	WeaponToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	WeaponToDrop->SetActorLocation(FinalLocation);
	WeaponToDrop->SetActorRotation(DropRotation);
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
	
	NewWeapon->SetWeaponState(EWeaponState::Equipped);
	NewWeapon->SetOwner(GetOwner());

	EWeaponSlot Slot = NewWeapon->WeaponType;

	// 무기의 타입에 맞는 슬롯이 비었는지 확인하고 비어 있지 않으면 해당 슬롯에 있는 무기를 떨어뜨림
	if (CarriedWeapons.Contains(Slot)) {
		ABaseWeapon* OldWeapon = CarriedWeapons[Slot];

		if (CurrentWeapon == OldWeapon) CurrentWeapon = nullptr;

		if (OldWeapon) {
			OldWeapon->SetOwner(nullptr);
			OldWeapon->SetWeaponState(EWeaponState::Dropped);
			DropWeaponSafeLocation(OldWeapon);
		}
	}

	// 무기 등록
	CarriedWeapons.Add(Slot, NewWeapon);

	// 바로 장착
	EquipWeaponBySlot(Slot);

	// 현재 들고 있는 무기가 없으면 바로 장착
	//if (CurrentWeapon == nullptr) {
	//	EquipWeaponBySlot(Slot);
	//}
	//else {
	//	// 당장 사용하지 않을 시 숨김
	//	NewWeapon->SetActorHiddenInGame(true);
	//	NewWeapon->SetActorEnableCollision(false);
	//}
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

	if (OnCurrentWeaponChanged.IsBound()) {
		OnCurrentWeaponChanged.Broadcast(CurrentWeapon);
	}

	AttachWeaponToHand(CurrentWeapon);
}

void UCombatComponent::AttachWeaponToHand(ABaseWeapon* Weapon)
{
	if (!Weapon || !GetOwner()) return;

	AUnrealProjectCharacter* Owner = Cast<AUnrealProjectCharacter>(GetOwner());
	if (Owner && Weapon) {
		Weapon->AttachToComponent(
			Owner->GetMesh1P(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("GripPoint") /* 캐릭터 메쉬의 손 소켓 이름 */
		);
	}

	Weapon->SetActorRelativeLocation(FVector::ZeroVector);
	Weapon->SetActorRelativeRotation(FRotator::ZeroRotator);
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
