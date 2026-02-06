// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealProjectCharacter.h"
#include "UnrealProjectPlayerController.h"
#include "../UnrealProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Component/CombatComponent.h"
#include "../Component/DroneComponent.h"
#include "../Component/AttributeComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUnrealProjectCharacter

AUnrealProjectCharacter::AUnrealProjectCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	DroneComponent = CreateDefaultSubobject<UDroneComponent>(TEXT("DroneComp"));

	if (GetCharacterMovement()) {
		NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
		DownedSpeed = 100.0f;
		TargetSpeed = NormalWalkSpeed;
		SprintSpeed = NormalWalkSpeed * 1.7f;
		GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
		GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	}
}

void AUnrealProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());

	//if (CrosshairWidgetClass && IsLocallyControlled()) // 내 캐릭터일 때만
	//{
	//	CrosshairWidget = CreateWidget<UUserWidget>(PC, CrosshairWidgetClass);
	//	if (CrosshairWidget)
	//	{
	//		CrosshairWidget->AddToViewport();
	//	}
	//}

	if (AttributeComponent) {
		AttributeComponent->OnDeath.AddDynamic(this, &AUnrealProjectCharacter::Downed);
	}

	if (DroneComponent)
	{
		DroneComponent->OnReviveComplete.AddDynamic(this, &AUnrealProjectCharacter::Revive);
	}

	GetMesh()->HideBoneByName(TEXT("head"), PBO_None);

	/*if (AttributeComponent) {
		AttributeComponent->OnDeath.AddDynamic(this, &AUnrealProjectCharacter::Death);
	}*/
}

void AUnrealProjectCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()) {
		float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
		
		// 플레이어 속도를 자연스럽게 증가/하락시키기 위한 보간
		float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, 2.0f);

		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}
}

void AUnrealProjectCharacter::Input_EquipPrimary()
{
	if (CombatComponent == nullptr) return;
	CombatComponent->EquipWeaponBySlot(EWeaponSlot::Primary);
}

void AUnrealProjectCharacter::Input_EquipSecondary()
{
	if (CombatComponent == nullptr) return;
	CombatComponent->EquipWeaponBySlot(EWeaponSlot::Secondary);
}

void AUnrealProjectCharacter::Input_EquipMelee()
{
	if (CombatComponent == nullptr) return;
	CombatComponent->EquipWeaponBySlot(EWeaponSlot::Melee);
}

void AUnrealProjectCharacter::Input_EquipThrowable()
{
	if (CombatComponent == nullptr) return;
	CombatComponent->EquipWeaponBySlot(EWeaponSlot::Throwable);
}

//////////////////////////////////////////////////////////////////////////// Input

void AUnrealProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, CombatComponent, &UCombatComponent::Attack);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, CombatComponent, &UCombatComponent::StopAttack);

		// Reloading
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, CombatComponent, &UCombatComponent::Reload);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Move);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AUnrealProjectCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AUnrealProjectCharacter::StopSprint);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AUnrealProjectCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AUnrealProjectCharacter::StopCrouch);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Look);

		// WeaponEquip/Change
		EnhancedInputComponent->BindAction(EquipPrimaryAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Look);
		EnhancedInputComponent->BindAction(EquipSecondaryAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Look);
		EnhancedInputComponent->BindAction(EquipMeleeAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Look);
		EnhancedInputComponent->BindAction(EquipThrowableAction, ETriggerEvent::Triggered, this, &AUnrealProjectCharacter::Look);

		// DroneAction
		EnhancedInputComponent->BindAction(DroneActiveSkillAction, ETriggerEvent::Triggered, DroneComponent, &UDroneComponent::ActiveDroneSkill);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

bool AUnrealProjectCharacter::IsWeaponEquipped() const
{
	return (CombatComponent && CombatComponent->GetCurrentWeapon() != nullptr);
}

void AUnrealProjectCharacter::GetHit_Implementation(const FVector& ImpactPoint)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Hit"));
}

void AUnrealProjectCharacter::Revive(float RevivePercent)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Revive"));
	SetPlayerState(EPlayerState::EPS_Normal);

	if (AttributeComponent) {
		// 최대 체력의 일부만큼 회복
		float ReviveAmount = AttributeComponent->GetMaxHealth() * (RevivePercent / 100.0f);

		AttributeComponent->Heal(ReviveAmount);
	}
}

void AUnrealProjectCharacter::Downed(AActor* VictimActor, AActor* KillerActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Downed"));
	SetPlayerState(EPlayerState::EPS_Downed);

	// 드론 부활 시도
	if (DroneComponent) {
		if (!DroneComponent->TryActivateRevive()) {
			Death(KillerActor);
		}
	}
}

void AUnrealProjectCharacter::Death(AActor* KillerActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Death"));
	SetPlayerState(EPlayerState::EPS_Dead);

	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("head") // 메쉬의 머리 뼈 이름
		);
	}

	if (OnDeath.IsBound()) {
		OnDeath.Broadcast(this, KillerActor);
	}
}

void AUnrealProjectCharacter::SetPlayerState(EPlayerState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;

	if (AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(GetController())) {
		PC->ChangeInputContext(CurrentState == EPlayerState::EPS_Downed);
	}

	switch (CurrentState) {
	case EPlayerState::EPS_Normal:
		// 정상 속도
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
		TargetSpeed = NormalWalkSpeed;
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
		break;
	case EPlayerState::EPS_Downed:
		// 속도 변경
		GetCharacterMovement()->MaxWalkSpeed = DownedSpeed;
		TargetSpeed = DownedSpeed;
		GetCapsuleComponent()->SetCapsuleHalfHeight(40.0f);

		break;
	case EPlayerState::EPS_Dead:
		// 모든 조작 차단
		DisableInput(Cast<APlayerController>(GetController()));

		// 랙돌(Ragdoll) 실행
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetMesh1P()->SetSimulatePhysics(true);
		GetMesh1P()->SetCollisionProfileName(TEXT("Ragdoll"));
		break;
	}
}


void AUnrealProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AUnrealProjectCharacter::StartSprint()
{
	// Shift pressed 
	TargetSpeed = SprintSpeed;
	bIsSprinting = true;
}

void AUnrealProjectCharacter::StopSprint()
{
	// Shift released
	TargetSpeed = NormalWalkSpeed;
	bIsSprinting = false;
}

void AUnrealProjectCharacter::StartCrouch()
{
	if (GetCharacterMovement()) {
		this->Crouch();
	}
}

void AUnrealProjectCharacter::StopCrouch()
{
	if (GetCharacterMovement()) {
		this->UnCrouch();
	}
}

void AUnrealProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUnrealProjectCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 떨어질때 속도 체크
	float FallingSpeed = -GetVelocity().Z;

	float VolumeMultiplier = FMath::Clamp(FallingSpeed / 1000.0f, 0.2f, 1.0f);

	// 떨어진 높이만큼 소리 크게
	UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation(), VolumeMultiplier);
	MakeNoise(VolumeMultiplier, this, GetActorLocation());
}
