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

	CarriedAmmoFill();

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


void UCombatComponent::CarriedAmmoFill()
{
	CarriedAmmo.Emplace(EAmmoType::EAT_AssaultRifle, MaxARAmmo);
	CarriedAmmo.Emplace(EAmmoType::EAT_Launcher, MaxLCAmmo);
	CarriedAmmo.Emplace(EAmmoType::EAT_Shotgun, MaxSGAmmo);
	CarriedAmmo.Emplace(EAmmoType::EAT_Sniper, MaxSRAmmo);

	if (ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon))
	{
		EAmmoType AmmoType = RangedWeapon->WeaponAmmoType;
		int32 AvailableAmmo = CarriedAmmo.Contains(AmmoType) ? CarriedAmmo[AmmoType] : -1;

		OnReserveAmmoChanged.Broadcast(AvailableAmmo);
	}
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

	// 투척 특별 관리
	if (Slot == EWeaponSlot::Throwable && CarriedWeapons.Contains(Slot)) {
		ABaseWeapon* OldWeapon = CarriedWeapons[Slot];

		// 들고 있는 것과 같은 클래스일 시
		if (OldWeapon->GetClass() == NewWeapon->GetClass()) {
			// 개수 증가
			ARangedWeapon* OldGrenade = Cast<ARangedWeapon>(OldWeapon);
			OldGrenade->PlusMaxAmmoInClip();
			OldGrenade->SetCurrentAmmoInClip(OldGrenade->GetMaxAmmoInClip());

			NewWeapon->Destroy();

			EquipWeaponBySlot(Slot);
			return;
		}
	}

	NewWeapon->SetWeaponState(EWeaponState::Equipped);
	NewWeapon->SetOwner(GetOwner());
	NewWeapon->SetInstigator(Cast<APawn>(GetOwner()));

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
	if (CurrentWeapon) {
		CurrentWeapon->SetActorHiddenInGame(true);

		if (ARangedWeapon* OldRangedWeapon = Cast<ARangedWeapon>(CurrentWeapon)) {
			OldRangedWeapon->CancelReload();
			OldRangedWeapon->OnWeaponReloadFinished.RemoveDynamic(this, &UCombatComponent::HandleWeaponReloadFinished);
		}
	}

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

	if (ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon))
	{
		EAmmoType AmmoType = RangedWeapon->WeaponAmmoType;
		int32 AvailableAmmo = CarriedAmmo.Contains(AmmoType) ? CarriedAmmo[AmmoType] : -1;

		if (OnReserveAmmoChanged.IsBound()) {
			OnReserveAmmoChanged.Broadcast(AvailableAmmo);
		}

		if (OnCurrentWeaponChanged.IsBound()) {
			OnCurrentWeaponChanged.Broadcast(CurrentWeapon);
		}

		RangedWeapon->OnWeaponReloadFinished.RemoveDynamic(this, &UCombatComponent::HandleWeaponReloadFinished);
		RangedWeapon->OnWeaponReloadFinished.AddDynamic(this, &UCombatComponent::HandleWeaponReloadFinished);
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
	if (CurrentWeapon)
	{
		CurrentWeapon->StopAttack();
	}
}

void UCombatComponent::Reload() {
	if (CurrentWeapon == nullptr) return;

	if (ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon)) {
		EAmmoType AmmoType = RangedWeapon->WeaponAmmoType;

		int32 AvailableAmmo = 0;
		if (CarriedAmmo.Contains(AmmoType))
		{
			AvailableAmmo = CarriedAmmo[AmmoType];
		}

		if (AvailableAmmo > 0) {
			RangedWeapon->Reload(AvailableAmmo);
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("This weapon cant Reload"));
}

void UCombatComponent::CycleWeapon(bool bScrollDown)
{
	uint8 CurrentSlotIndex = 0;

	if (CurrentWeapon) {
		CurrentSlotIndex = (uint8)CurrentWeapon->WeaponType;
	}

	uint8 NextSlotIndex = CurrentSlotIndex;
	uint8 MaxSlotIndex = (uint8)EWeaponSlot::MAX;

	for (int32 i = 0; i < MaxSlotIndex; i++) {
		// 휠 방향에 따라 인덱스 이동
		if (bScrollDown) {
			NextSlotIndex++;
			if (NextSlotIndex >= MaxSlotIndex) NextSlotIndex = 0;
		}
		else {
			if (NextSlotIndex == 0) NextSlotIndex = MaxSlotIndex - 1;
			else NextSlotIndex--;
		}

		EWeaponSlot SlotToCheck = (EWeaponSlot)NextSlotIndex;

		// 슬롯이 비어 있다면 다음 슬롯 검사
		if (CarriedWeapons.Contains(SlotToCheck) && CarriedWeapons[SlotToCheck] != nullptr)
		{
			// 무기를 찾았다면 바로 장착
			EquipWeaponBySlot(SlotToCheck);
			break;
		}
	}
}

void UCombatComponent::HandleWeaponReloadFinished(int32 AmmoConsumed)
{
	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(CurrentWeapon);
	if (!RangedWeapon) return;

	EAmmoType AmmoType = RangedWeapon->WeaponAmmoType;

	if (AmmoConsumed > 0)
	{
		CarriedAmmo[AmmoType] -= AmmoConsumed;

		OnReserveAmmoChanged.Broadcast(CarriedAmmo[AmmoType]);
	}
}

void UCombatComponent::DiscardEmptyWeapon()
{
	if (CurrentWeapon) {
		CarriedWeapons.Remove(CurrentWeapon->WeaponType);

		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;

		EquipWeaponBySlot(EWeaponSlot::Primary);
	}
}
