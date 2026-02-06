// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "../HitInterface.h"
#include "UnrealProjectCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UCharacterMovementComponent;
class UCombatComponent;
class UAttributeComponent;
class UDroneComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathPlayer, AActor*, Victim, AActor*, Killer);

UENUM(BlueprintType)
enum class EPlayerState : uint8 {
	EPS_Normal UMETA(DisplayName = "Normal"), // 정상
	EPS_Downed UMETA(DisplayName = "Downed"), // 기절
	EPS_Dead UMETA(DisplayName = "Dead"), // 사망
};

UCLASS(config=Game)
class AUnrealProjectCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	// 사격 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	// 장전 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// 달리기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	// 앉기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	// 주무기로 바꾸기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipPrimaryAction;

	// 보조무기로 바꾸기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSecondaryAction;

	// 근접무기로 바꾸기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipMeleeAction;

	// 투척무기로 바꾸기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipThrowableAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DroneActiveSkillAction;

	//// 크로스헤어를 넣어줄 변수
	//UPROPERTY(EditAnywhere, Category = "UI")
	//TSubclassOf<class UUserWidget> CrosshairWidgetClass;

	//// 생성된 위젯을 담아둘 변수
	//UPROPERTY()
	//class UUserWidget* CrosshairWidget;
	
public:
	AUnrealProjectCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

protected:

	void Input_EquipPrimary();
	void Input_EquipSecondary();
	void Input_EquipMelee();
	void Input_EquipThrowable();

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for movement input */
	void StartSprint();

	/** Called for movement input */
	void StopSprint();

	/** Called for movement input */
	void StartCrouch();

	/** Called for movement input */
	void StopCrouch();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Landed(const FHitResult& Hit) override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsWeaponEquipped() const;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	UFUNCTION()
	void Revive(float RevivePercent);

	// 누웠을 때 실행할 함수
	UFUNCTION()
	void Downed(AActor* VictimActor, AActor* KillerActor);

	// 죽었을 때 실행할 함수
	UFUNCTION()
	void Death(AActor* KillerActor);

	// 상태 확인용
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDowned() const {
		return CurrentState == EPlayerState::EPS_Downed;
	}

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDead() const {
		return CurrentState == EPlayerState::EPS_Dead;
	}

private:
	// 걷기 속도
	float NormalWalkSpeed;
	float DownedSpeed;
	
	// 중간 보간을 위한 목표값
	float TargetSpeed;

protected:
	// 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState = EPlayerState::EPS_Normal;

	// 상태 변경
	void SetPlayerState(EPlayerState NewState);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// 달리기 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Walking", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float SprintSpeed;

	// 착지 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Landed")
	USoundBase* LandSound;

	// 무기를 줍거나 버리는 등의 무기 상호작용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComponent;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttributeComponent* AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneComponent* DroneComponent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathPlayer OnDeath;
};

