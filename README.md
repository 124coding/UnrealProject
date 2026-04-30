# [C++ / Unreal Engine] Unreal Project

Unreal Engine 5와 C++을 기반으로 개발된 1인칭 PVE 서바이벌 슈팅 게임입니다.
레프트 4 데드(Left 4 Dead), GTFO와 같은 웨이브 디펜스 및 돌파형 게임을 모작하며, 객체 지향 설계(OOP), 대규모 객체 최적화(Object Pooling), 데이터 주도(Data-Driven) 아키텍처 등 클라이언트 프로그래머로서의 기술적 고민과 트러블슈팅 과정을 담았습니다.

**게임명:** Unreal 1인칭 PVE  
**개발 환경:** C++, Unreal Engine 5.4.4  
**개발 기간 & 인원:** 2026/1 ~ 2026/4 (4개월), 1인 개발  
**핵심 역할:** 오브젝트 풀링 구현, 최적화, 인게임 로직 전반

<video src="https://github.com/user-attachments/assets/38953900-a5dd-4ef9-ae70-6101a6a8f0ff" muted playsinline width="100%"></video>  

[[Unreal Project 시연 영상]](https://www.youtube.com/watch?v=Qs79UwB2QN0)

## 시스템 아키텍처 및 게임 루프

### 1. 초기화 단계 (Initialization):
- GameMode: 게임의 규칙을 정의하고 전체 시스템을 가동합니다. BeginPlay 시점에 GameState를 확인하고 AI 디렉터를 활성화합니다.  

- GameState: 전역 저장소로 오브젝트 풀(Object Pool)을 초기화합니다. 적, 총알, 데미지 텍스트 등 빈번한 생성이 필요한 객체들을 미리 메모리에 적재하여 런타임 오버헤드를 방지합니다.  

### 2. 전투 루프 (Gameplay Loop):
- AI 디렉터 (in GameMode): 실시간으로 토큰(Token)을 생성하며 게임의 텐션을 조절합니다. 예산이 확보되면 GameState의 풀에 적 스폰을 요청합니다.  

- Pawn/Character: CombatComponent 와 AttributeComponent를 통해 전투를 수행합니다. 피격 시 방향을 계산하고, 체력이 0이 되면 OnDeath 델리게이트를 방송(Broadcast)합니다.  

- Object Pooling: 사망한 적이나 충돌한 총알은 파괴되지 않고 ReturnToPool을 통해 비활성화되어 다음 스폰을 대기합니다.  

### 3. 결과 및 전환 (End Game):
- GameMode: 클리어 트리거를 밟게 되면 게임 클리어 UI를 컨트롤러보고 띄우라고 요청합니다.   

- PlayerController: GameMode로부터 신호를 받아 마우스 커서를 활성화하고 결과 UI를 화면에 띄우는 등 '유저 인터페이스' 제어를 전담합니다.  



## 핵심 아키텍처 및 구현 로직 (Key Features)

### 1. 전역 오브젝트 풀링 (Centralized Object Pooling) 

개발 초기에는 몬스터와 투사체가 등장할 때마다 동적 할당(SpawnActor)과 해제(Destroy)를 반복하는 구조로 구현했습니다. 하지만 대규모 웨이브 진행 시 빈번한 메모리 할당과 가비지 컬렉션(GC) 스파이크로 인해 치명적인 런타임 오버헤드와 프레임 드랍이 발생할 것을 우려했습니다. 이를 근본적으로 해결하고 메모리 파편화를 방지하기 위해 중앙 집중식 풀링 시스템을 도입했습니다.

- GameMode 중앙 관리 (O(1) 탐색): 플레이어에 종속되는 무기 풀을 제외한 대다수의 액터(적, 총알 등)가 개별적인 풀을 가지는 대신, 게임의 룰을 총괄하는 GameMode와 전역 상태인 GameState가 TMap 자료구조를 활용해 모든 풀을 통제하도록 통합했습니다. 이를 통해 어떤 객체든 O(1)의 속도로 탐색하여 대여/반납할 수 있어 런타임 동적 할당 비용을 최소화했습니다.

- 완전 동면(Stasis) 모드: 풀로 반환된 객체는 SetMovementMode(MOVE_None) 및 GravityScale = 0을 적용해 물리 엔진과의 연결을 끊어 백그라운드에서의 CPU 점유율을 0으로 수렴시켰습니다.  

- 인터페이스(Interface) 기반 제어: IObjectPoolInterface를 도입해 몬스터, 총알, 3D 데미지 UI 텍스트 등 형태가 다른 모든 객체가 단일한 풀 매니저 안에서 재활용될 수 있도록 하였습니다.

<details>
<summary>Object Pool Component</summary>
<div markdown="1">

```C++
// Object Pool Component
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UObjectPoolComponent();
	

public:	
	// 풀 초기화 (어떤 클래스를 몇 개 만들건지)
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void InitializePool(TSubclassOf<AActor> ClassToPool, int32 Count, AActor* Owner = nullptr);

	// 풀에서 하나만 가져오기
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* SpawnFromPool(FVector SpawnLocation, FRotator SpawnRotation);

	// 풀로 돌려보내기
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnToPool(AActor* ActorToReturn);

	// 모든 액터 풀로 돌려보내기
	void ReturnAllToPool();

protected:
	UPROPERTY()
	TSubclassOf<AActor> PooledClass;

	// 대기 중인 오브젝트들
	UPROPERTY()
	TArray<AActor*> PoolQueue;

	// 풀 확장 (부족할 시 추가 생성)
	AActor* CreateNewObject();
};
```
</div>
</details>  
</br>

<details>
<summary>PoolableInterface</summary>
<div markdown="1">

```C++
// PoolableInterface
#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PoolableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

class UNREALPROJECT_API IPoolableInterface
{
	GENERATED_BODY()

public:
	// 현재 풀에서 활성화되어 사용 중인지 확인하는 함수
	virtual bool IsActiveInPool() const = 0;

	// 풀 활성화/비활성화 상태를 세팅하는 함수
	virtual void SetActiveInPool(bool bActive) = 0;

	// 풀에서 꺼내질 때 (SetActive) - 변수 초기화, 물리 켜기
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnPoolSpawned();

	// 풀로 돌아갈 때 (SetInactive) - 물리 끄기, 숨기기
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnPoolReturned();
	
	// 주인 Pool에 넣는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void SetOwningPool(UObjectPoolComponent* NewPool);
};
```
</div>
</details>  
</br>

### 2. 모듈화된 무기 체계 및 전투 아키텍처 (Component & OOP)

처음 개발을 시작했을때에는 캐릭터 클래스 내부에 전투 로직과 피격 판정, 무기 관리 코드 등 모든 것이 혼재되어 있었습니다. 또한 무기 종류(근접, 원거리, HitScan 등)가 늘어날수록 발사 조건을 체크하는 if문이 중첩되어, 스파게티 코드로 변질될 위험이 컸으며 유지보수에 대한 부담감이 커져만 갔습니다. 이를 해결하고자 기능의 강한 결합을 끊어내고, 클래스 상속과 컴포넌트 패턴을 활용하여 전투 아키텍처를 설계했습니다.  

- 템플릿 메서드 패턴(Template Method Pattern) 적용: 무기의 최상위 클래스(ABaseWeapon)에 공격의 전체 흐름을 통제하는 '검문소 패턴'을 도입하여 발사 명령(Attack), 발사 조건 검증(CanAttack), 실제 발사(OnAttack)으로 책임을 분리하였습니다. 이를 통해 하위 클래스들은 자신에게 필요한 로직만 다형성(Polymorphism)을 활용해 오버라이딩하게 만들어, 기존 코드를 수정하지 않고도 새로운 무기를 추가할 수 있는 OCP(개방-폐쇄 원칙) 구조를 달성했습니다.  

- TMap 기반 지능형 인벤토리 및 모듈러 스왑: 무기 슬롯을 배열(Array)이 아닌 TMap<EWeaponSlot, ABaseWeapon*>으로 관리하여 O(1)의 속도로 무기에 접근하도록 설계하였습니다. 마우스 휠 스왑 시 비어있는 슬롯을 자연스럽게 건너뛰는 모듈러(Wrap-Around) 순회 알고리즘을 구현했으며, 수류탄과 같은 투척 무기 중복 획득 시 슬롯을 덮어쓰지 않고 탄약(Ammo)만 안전하게 합산하도록 예외 처리 로직을 작성하였습니다.  

- 컴포넌트 주도(Component-Driven) 범용 넉백 시스템: 기존 적(Enemy) 클래스 내부에 하드코딩 되어 있던 넉백 물리 연산을 독립적인 AttributeComponent로 완전히 분리했습니다. 이제 어떤 액터라도 이 컴포넌트를 부착하기만 하면 넉백 대상이 되며, 물리적 충돌과 AI 내비게이션 관성이 꼬이는 것을 막기 위해 임펄스(Impulse) 적용 직전 StopMovementImmediately를 호출하여 적이 날아가지 않는 버그 상황을 방지하였습니다.

<details>
<summary>Weapon의 Attack 분리 코드</summary>
<div markdown="1">

```C++
void ABaseWeapon::Attack() {
	if (!CanAttack()) return;

	OnAttack();
}

bool ABaseWeapon::CanAttack()
{
	if (CurrentState == EWeaponState::Dropped) return false;

	// 무기를 소유한 플레이어가 없으면 실행 불가
	if (!GetOwner()) return false;

	// 시간 계산해서 공격 속도 체크
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < CurrentWeaponStat.AttackRate) {
		UE_LOG(LogTemp, Log, TEXT("Cant Attack So Fast"));
		return false;
	}

	return true;

}

void ABaseWeapon::OnAttack()
{
	if (CurrentWeaponStat.FireMode == EWeaponFireMode::Single)
	{
		// 단발 (Single)
		ExecuteFire();
	}
	else if (CurrentWeaponStat.FireMode == EWeaponFireMode::Auto)
	{
		// 연사 (Auto)
		ExecuteFire();
		GetWorldTimerManager().SetTimer(
			FireTimerHandle, this, &ABaseWeapon::ExecuteFire, CurrentWeaponStat.AttackRate, true
		);
	}
	else if (CurrentWeaponStat.FireMode == EWeaponFireMode::Burst)
	{
		// 점사 (Burst)
		CurrentBurstCount = 0;
		HandleBurstFire();
	}
}

void ABaseWeapon::StopAttack() {

	if (CurrentWeaponStat.FireMode == EWeaponFireMode::Auto)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
}
```
</div>
</details>  
</br>


<details>
<summary>AttributeComponent를 통한 체력 관리 및 넉백 적용 코드</summary>
<div markdown="1">

```C++
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
```
</div>
</details>  
</br>

<img width="500" height="450" alt="Weapon Diagram" src="https://github.com/user-attachments/assets/bde3fa67-c1a4-4efb-b3f7-44bd691c7ddb" />
Weapon Diagram

### 3. AI 디렉터 기반 동적 웨이브 및 팝인 방지

하드코딩된 위치와 수량으로 적으로 스폰하는 전통적인 방식은 플레이어가 패턴을 쉽게 암기하게 만들어 게임의 리플레이 가치를 떨어뜨립니다. 또한, 플레이어 눈앞에서 적이 갑자기 생성되는 팝인 (Pop-in) 현상은 게임의 몰입감을 훼손하는 문제가 있다고 판단하여 이를 해결하기 위해 수학적 연산과 토큰 경제(Token Economy) 개념을 결합한 지능형 디렉터 시스템을 구축했습니다.

- 토큰(Token) 경제 기반의 동적 스폰 알고리즘: 고정된 웨이브 데이터를 버리고, AI 디렉터가 매초 예산(Token)을 비축하도록 설계했습니다. 디렉터는 현재의 페이즈(BuildUp, Peak 등)에 따라 예산을 소비하며, 가중치 확률(Weight-based Random) 기반으로 적의 조합을 실시간으로 구성(SpendTokensToSpawn)합니다. 이를 통해 소수의 원거리 적과 근거리 적을 스폰하거나 다수의 근거리 적이 스폰되는 등 예측 불가능한 전투 텐션을 가능케 했습니다.  

- 2D 뷰포트 투영 및 NavMesh 기반 무결성 스폰: 팝인(Pop-in) 현상을 막기 위해 3단계 검증 알고리즘을 도입했습니다. 첫째, 무작위 스폰 좌표를 NavMesh에 투영하여 갈 수 있는 길인지 확인, 둘째, 카메라와의 LineTrace로 물리적 은폐를 검사합니다. 셋째, 3D 내적(Dot Product)의 사각지대를 극복하기 위해 3D 좌표를 2D 모니터 픽셀 위치로 변환(ProjectWorldLocationToScreen)하여 플레이어의 실제 화면 밖임이 수학적으로 증명되었을때만 스폰을 허가 하였습니다. 그러나 SpawnVolume을 플레이어가 전부 보고 있는 경우도 있을 수 있어 불가피한 사각지대가 없을 경우를 대비해 가장 안전한 위치로 스폰을 유도하는 Fallback(대체) 메커니즘을 구현하여 시스템의 안정성을 높였습니다.  

- 스마트 링크(Smart Link) 기반 3D 포물선 점프 내비게이션: AI가 평면 이동의 한계를 벗어나 입체적인 추적을 할 수 있도록 NavLinkProxy를 확장했습니다. AI가 낭떠러지나 단차에 도달하면 SuggestProjectileVelocity 함수를 이용해 목표 착지점까지의 최적 발사 벡터(Velocity)를 엔진 중력 값에 맞춰 역산합니다. 이후 LaunchCharacter로 물리적인 힘을 가해 강제로 도약 가능하게 하였습니다.  

<details>
<summary>Token을 통한 Spawn 코드</summary>
<div markdown="1">

```C++
void AUnrealProjectGameMode::SpendTokensToSpawn(int32 MaxTokensToSpend, const TArray<FEnemySpawnInfo>& SpawnList, AActor* TargetActor)
{
	int32 TokensLeft = MaxTokensToSpend;
	int32 Failsafe = 50; // 무한 루프 방지용 (최대 50번만 구매 시도)

	while (TokensLeft > 0 && Failsafe > 0) {
		Failsafe--;

		// 살 수 있는 몬스터만 필터링
		TArray<FEnemySpawnInfo> AffordableEnemies;
		for (const FEnemySpawnInfo& Info : SpawnList) {
			if (Info.SpawnCost <= TokensLeft) {
				AffordableEnemies.Add(Info);
			}
		}

		// 살 수 있는 몬스터가 없다면 종료
		if (AffordableEnemies.Num() == 0) break;

		TSubclassOf<AActor> SelectedClass = GetRandomEnemyClass(AffordableEnemies);

		if (SelectedClass) {
			
			// 방금 뽑은 클래스의 비용
			int32 CostToDeduct = 0;
			for (const FEnemySpawnInfo& Info : AffordableEnemies) {
				if (Info.EnemyClass == SelectedClass) {
					CostToDeduct = Info.SpawnCost;
					break;
				}
			}

			// 비용 차감
			TokensLeft -= CostToDeduct;
			CurrentDirectorTokens -= CostToDeduct;

			SpawnEnemyInGroup(CurrentActiveGroupID, 1, SelectedClass, TargetActor);
		}
	}
}
```

</div>
</details>  
</br>

<details>
<summary>SpawnVolume의 적 스폰 가능 위치 찾기 코드</summary>
<div markdown="1">

```C++
FVector ASpawnVolume::GetRandomPointInVolume()
{
	FVector Origin = SpawningBox->GetComponentLocation();
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	const int32 MaxAttempts = 10; // 최대 10번 시도

	for (int32 i = 0; i < MaxAttempts; i++) {
		// 박스 안의 랜덤 위치
		FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

		// NavMesh 투영
		FNavLocation NavLoc;
		const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		if (NavSystem && NavSystem->ProjectPointToNavigation(RandomPoint, NavLoc)) {
			FVector CandidateLoc = NavLoc.Location;
			CandidateLoc.Z += 100.0f;

			// 스폰 위치에 장애물이 있는지 확인
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			bool bHit = GetWorld()->OverlapBlockingTestByChannel(
				CandidateLoc,
				FQuat::Identity,
				ECollisionChannel::ECC_Visibility,
				FCollisionShape::MakeSphere(50.0f), /* 현재 하드 코딩으로 50.0f 값이 들어가 있기에 추후 적의 크기에 맞춰서 수정 필요 */
				Params
			);

			// 충돌이 없으면 플레이어 시야 검사
			if (!bHit) {
				APlayerController* PC = GetWorld()->GetFirstPlayerController();

				if (PC && PC->GetPawn()) {
					FVector CameraLoc;
					FRotator CameraRot;
					PC->GetPlayerViewPoint(CameraLoc, CameraRot);

					FHitResult LineHit;
					FCollisionQueryParams LineParams;
					LineParams.AddIgnoredActor(this);
					LineParams.AddIgnoredActor(PC->GetPawn()); // 플레이어는 무시

					// 카메라 위치에서 스폰 지점으로 RayCast
					bool bHitWall = GetWorld()->LineTraceSingleByChannel(
						LineHit,
						CameraLoc,
						CandidateLoc,
						ECC_Visibility,
						LineParams
					);

					if (bHitWall) {
						return CandidateLoc;
					}
					else {
						// 모니터 화면 내에 보이는 위치인지 판별
						FVector2D ScreenPosition;

						// 스폰 좌표를 모니터 화면 좌표로 변환
						bool bIsInFrontOfCamera = PC->ProjectWorldLocationToScreen(CandidateLoc, ScreenPosition);

						if (bIsInFrontOfCamera) {
							int32 ViewportSizeX, ViewportSizeY;
							PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

							// 여유 공간을 줘서 화면 밖 100픽셀 밖에서 스폰 보정
							float Margin = 100.f;

							bool bIsOnScreen = (ScreenPosition.X >= -Margin && ScreenPosition.X <= ViewportSizeX + Margin) &&
											   (ScreenPosition.Y >= -Margin && ScreenPosition.Y <= ViewportSizeY + Margin);

							if (bIsOnScreen) continue;
							else return CandidateLoc;
						}
					}
				}
				else
				{
					// 플레이어를 못 찾았다면 그냥 스폰
					return CandidateLoc;
				}
			}
		}
	}

	return Origin; // 실패 시 중앙
}
```

</div>
</details>  
</br>

### 4. 전술적 AI (Behavior Tree & EQS) 및 전투 피드백(Animation) 고도화
단순히 플레이어를 향해 직선으로 달려오기만 하는 몬스터는 1인칭 슈팅 게임의 긴장감을 떨어뜨립니다. 또한, 타격과 피격의 시각적 피드백이 명확하지 않으면 게임의 타격감이 훼손됩니다. 이를 해결하기 위해 언리얼 엔진의 AI 시스템을 적극 활용하여 스스로 판단하는 전술적 AI를 설계하고, 애니메이션 파이프라인을 튜닝하여 전투 피드백을 구현했습니다.  

- EQS와 Behavior Tree를 결합한 지능형 추적: 원거리 적의 경우 플레이어와 무작정 거리를 좁히는 대신, EQS(Environment Query System)를 활용해 플레이어와의 적정 거리를 유지하며 시야가 확보되는 최적의 공격 포인트를 스스로 탐색하도록 구현했습니다. Behavior Tree를 통해 평상 시(Patrol), 추적(Chase), 공격(Attack) 등 AI의 상태(FSM)를 시각적이고 체계적으로 제어했습니다.  

- AnimNotify를 활용한 타격 판정: 공격 애니메이션과 실제 데미지 적용 타이밍이 어긋나는 것을 막기 위해 AnimNotify를 활용하였습니다. 적이 팔을 휘두르는 공격 시 애니메이션에서 임팩트를 줄 수 있다고 판단한 부분에 AnimNotify를 두고 데미지를 입히는 방식을 채택했습니다.  

- 우선순위 기반 애니메이션 레이어링 (Animation Layering): 적이 공격 모션을 취하는 도중에 피격 당할 경우, 애님 그래프(AnimGraph)에서 로직을 강제 종료하는 대신 블렌딩 우선순위를 조정(Attack Slot -> Hit Slot)하여 부드럽게 피격 리액션이 덮어씌워지도록(Override) 설계하여 시각적인 어색함을 제거했습니다.

<details>
<summary>AnimNotify 코드 및 적의 근접 공격 코드</summary>
<div markdown="1">

```C++
// AnimNotify 호출 함수
void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner()) {
		// 이 몽타주의 주인이 BaseEnemy인지 확인
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(MeshComp->GetOwner());

		if (Enemy) {
			Enemy->PerformMeleeAttackHitCheck(SocketName, HalfRadiusSize, DamageMultiplier);
		}
	}
}

// 적의 근접 공격 함수
void ABaseEnemy::PerformMeleeAttackHitCheck(FName SocketName, float HalfRadiusSize, float DamageMultiplier)
{
	// 트레이스 시작 위치
	FVector Start = GetMesh()->GetSocketLocation(SocketName);
	FVector End = Start + (GetActorForwardVector() * 20.0f);

	// 충돌 대상 설정 (Player만 감지하도록)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Player));

	// 자기 자신은 무시
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	FHitResult OutHit;

	bool bResult = UKismetSystemLibrary::SphereTraceSingleForObjects(
		GetWorld(),
		Start,
		End,
		HalfRadiusSize,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration, // 디버그용(추후 None으로 변경)
		OutHit,
		true
	);

	if (bResult) {
		AActor* HitActor = OutHit.GetActor();

		// 플레이어인지 확인 후 데미지
		if (HitActor) {
			UGameplayStatics::ApplyDamage(
				HitActor,
				CurrentEnemyStat.Damage * DamageMultiplier,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}
	}
}
```

</div>
</details>  
</br>

<img width="1200" height="500" alt="Ranged Enemy BT" src="https://github.com/user-attachments/assets/43957055-355f-4d31-9ee1-081ca31dfddc">
원거리 적의 BT(왼쪽부터 공격, 거리 조절, 너무 먼 거리일때는 Target에게로 직접 이동)  

<img width="850" height="240" alt="Enemy Anim Graph" src="https://github.com/user-attachments/assets/06404163-8e3b-428e-9a05-f9f8139224da" />
블렌딩 우선순위


### 5. 데이터 주도 설계(Data-Driven)와 안전한 프레임워크 생명주기
몬스터의 체력, 무기의 데미지 등 기획적인 수치가 C++ 코드에 하드코딩되어 있으면 밸런스 수정 시 매번 엔진을 다시 빌드해야 하는 비효율이 발생합니다. 또한, 레벨(맵)이 전환되거나 게임을 재시작할 때 휘발성 데이터와 영구 보존 데이터가 꼬여 세이브 파일이 날아가거나 크래시가 발생하는 생명주기(Lifecycle) 문제가 있었습니다. 이를 데이터 주도 아키텍처와 권한 분리로 해결했습니다.  

- DataTable과 구조체(Struct)를 활용한 데이터 일원화: 무기의 스탯, 드론의 업그레이드 수치, 몬스터 정보 등을 모두 DataTable로 분리했습니다. 프로그래머의 개입 없이 기획자가 외부 JSON이나 CSV 파일 수정만으로 게임의 밸런스를 즉시 조절할 수 있는 OCP(개방-폐쇄 원칙) 기반의 파이프라인을 확립했습니다.  

- GameInstance의 세이브 데이터 무결성 검증: 레벨 전환 시에도 유지되어야 하는 데이터는 GameInstance에 보관하되, 빈 데이터로 기존 세이브 파일이 덮어씌워지는 치명적 버그를 막기 위해 bIsSaveDataValid 플래그를 도입했습니다. 명시적인 상태 검증 절차를 통과했을 때만 데이터 병합을 허가하는 파이프라인을 구축해 데이터 유실을 차단했습니다.  

- MVC 패턴에 입각한 UI 및 게임 루프 분리: 게임 오버나 클리어 시, 게임의 룰을 통제하는 GameMode가 UI 위젯까지 직접 다루지 못하게 막았습니다. 룰 통제는 GameMode가, 화면 연출과 입력 모드(InputMode) 전환은 PlayerController가 담당하게 권한을 분리하여 추후 멀티플레이어 환경으로 확장하더라도 문제가 없도록 설계했습니다.  


<details>
<summary>BaseWeapon Data Struct</summary>
<div markdown="1">

```C++
USTRUCT(BlueprintType)
struct FBaseWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	EWeaponSlot WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float Damage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float AttackRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float AttackRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float KnockbackPower = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Base")
    EWeaponFireMode FireMode = EWeaponFireMode::Single;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    int32 MaxBurstCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    float BurstFireRate = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base|Assets")
    USoundBase* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base|Assets")
    UAnimMontage* FireAnimation;
};
```

</div>
</details>  
</br>

<details>
<summary>GameInstance를 통한 Save, Load</summary>
<div markdown="1">

```C++
void USurvivalGameInstance::ResetCampaignData()
{
	// 플레이어 스탯 (Attribute) 초기화
	PlayerAttributeData.MaxHealth = 100.f;
	PlayerAttributeData.CurrentHealth = 100.f;


	// 전투 및 인벤토리 (Combat) 초기화
	PlayerCombatData.SavedCarriedWeapons.Empty();
	PlayerCombatData.CurrentEquippedSlot = EWeaponSlot::MAX;
	PlayerCombatData.CarriedAmmo.Empty();

	// 드론 (Drone) 데이터 초기화
	SavedDroneStats = FDroneStats();
}

void USurvivalGameInstance::SaveGameToDisk(int SlotIndex)
{
	FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotIndex);

	USurvivalSaveGame* SaveObj = Cast<USurvivalSaveGame>(UGameplayStatics::CreateSaveGameObject(USurvivalSaveGame::StaticClass()));

	if (SaveObj) {
		// GI에 있는 데이터들을 SaveGame 객체로 복제
		SaveObj->SavedAttributeData = this->PlayerAttributeData;
		SaveObj->SavedCombatData = this->PlayerCombatData;
		SaveObj->SavedDroneStats = this->SavedDroneStats;
		SaveObj->SavedLevelName = this->SavedLevelName;

		// 하드에 .sav 파일로 굽기
		UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, SaveObj->UserIndex);
	}
}

void USurvivalGameInstance::LoadGameFromDisk(int SlotIndex)
{
	FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotIndex);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
		USurvivalSaveGame* LoadObj = Cast<USurvivalSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (LoadObj) {
			this->PlayerAttributeData = LoadObj->SavedAttributeData;
			this->PlayerCombatData = LoadObj->SavedCombatData;
			this->SavedDroneStats = LoadObj->SavedDroneStats;
			this->SavedLevelName = LoadObj->SavedLevelName;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Slot Empty"), *SlotName);
	}
}

void USurvivalGameInstance::ClearSaveData()
{
	FString SlotName = TEXT("Slot_1");

	// 슬롯 이름에 해당하는 세이브 파일을 하드디스크에서 삭제!
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	}
}
```

</div>
</details>  
</br>

<img width="1060" height="540" alt="HitScan DataTable" src="https://github.com/user-attachments/assets/aac8257c-efbd-4007-b9e1-fa8046e9cf73" />
HitScanWeapon DataTable 일부


## 트러블 슈팅

### 1. 재장전 중 무기 스왑 또는 픽업 시 크래쉬
- 문제 상황 (Problem): 원거리 무기(Ranged Weapon)을 재장전 중에 바닥의 무기를 줍거나 휠로 무기를 교체하면 언리얼 엔진이 강제 종료(Crash)되는 치명적인 버그가 있었습니다.  

- 원인 분석(Cause): 재장전은 일정 시간이 지난 후 탄약을 채우는 '비동기 타이머(Timer)'로 작동하는데 이때 무기를 스왑하게 되면 기존 장착 무기가 슬롯에서 빠지게 됩니다. 이때, 백그라운드에서 돌고 있던 타이머가 소멸된 무기의 메모리 주소(Dangling Pointer)에 접근하여 탄약 데이터를 갱신하려 시도했기에 발생한 참조 오류였습니다.  

- 해결 방법(Solution): 무기 장착 해제(Unequip) 및 스왑 로직 진입 시, 1순위로 진행 중인 모든 타이머를 강제 중단(ClearTimer)하는 방어 코드를 작성했습니다. 이후 CurrentWeapon 포인터를 명시적으로 초기화하여 잘못된 메모리 접근을 원천 차단한 뒤 다음 장착 로직으로 넘어가도록 안전한 포인터 생명주기(Lifecycle)를 확립하여 크래시를 해결하였습니다.  

<details>
<summary>Reload Crash 방어코드</summary>
<div markdown="1">

```C++
void UCombatComponent::EquipWeaponBySlot(EWeaponSlot SlotToEquip)
{
	/// ... (생략) ...

	// 현재 들고 있는 무기는 집어넣기
	if (CurrentWeapon) {
		CurrentWeapon->SetActorHiddenInGame(true);

        // 비동기 크래시 방지 방어 코드 (생명주기 동기화)
		if (ARangedWeapon* OldRangedWeapon = Cast<ARangedWeapon>(CurrentWeapon)) {
            // 실행 중인 장전 타이머 강제 취소
			OldRangedWeapon->CancelReload();

            // 댕글링 포인터 참조를 막기 위한 델리게이트 수신 차단 (Unbind)
			OldRangedWeapon->OnWeaponReloadFinished.RemoveDynamic(this, &UCombatComponent::HandleWeaponReloadFinished);
		}
	}

	/// ... (생략) ...
}
```

</div>
</details>  
</br>

### 2. 조작키 변경 무시
- 문제 상황 (Problem): 설정 UI에서 조작키를 변경하고 저장했음에도 불구하고, 실제 게임 내에서는 이전 키가 그대로 작동하거나 변경된 키가 무시되는 '입력 정합성 결함'이 발생했습니다. 이는 단순한 로직 오류가 아닌, 언리얼 엔진의 데이터 에셋 관리 방식과 입력 서브시스템 간의 구조적 불일치 문제였습니다.  

- 원인 분석(Cause): 
    1. 에셋의 불변성(Immutability): UInputMappingContext(IMC)는 디스크에 저장된 UDataAsset으로, 런타임 중에는 읽기 전용(ReadOnly)으로 보호됩니다. 일반적인 접근으로는 내부 매핑 데이터를 수정할 수 없었습니다.  

    2. 데이터 캐싱 및 갱신 지연: EnhancedInputSubsystem은 성능 최적화를 위해 IMC 데이터를 내부적으로 캐싱합니다. 에셋의 변수 값을 직접 수정하더라도, 서브시스템에 명시적인 재빌드(Rebuild) 명령을 내리지 않으면 런타임에 반영되지 않는 구조였습니다.  

    3. 데이터 파편화: '점프'나 '상호작용' 같은 액션이 상황별 IMC(Default, Downed 등)에 분산되어 있어, 단일 에셋 수정만으로는 전체 조작 무결성을 유지할 수 없었습니다.  

- 해결 방법(Solution): 
    1. const_cast를 통한 메모리 보호 해제: 엔진이 보호하는 const 속성을 강제로 해제하여 런타임 메모리에 로드된 IMC 에셋의 매핑 구조체(FEnhancedActionKeyMapping)에 직접 접근, 데이터를 갱신하는 통로를 확보했습니다.  

    2. 배치 업데이트(Batch Update) 시스템: 게임 내 존재하는 모든 IMC를 TArray로 수집하여, 단 한 번의 설정 변경으로 분산된 모든 에셋의 데이터를 일괄 동기화하는 로직을 구축했습니다.  

    3. 서브시스템 플러시(Flush): 데이터 수정 직후 RemoveMappingContext와 AddMappingContext를 순차적으로 호출하여 서브시스템이 수정된 에셋 데이터를 다시 캐싱하도록 강제함으로써 변경 사항을 즉시 반영했습니다.  

<details>
<summary>GameplaySetting Apply 코드</summary>
<div markdown="1">

```C++
void UGameplaySettingWidget::ApplyGameplaySetting()
{
	if (!DefaultIMC || !SaveData) return;

	AUnrealProjectPlayerController* PC = Cast<AUnrealProjectPlayerController>(GetOwningPlayer());
	if (!PC) return;

    // 마우스 감도 등 즉시 반영 가능한 스탯 갱신
	if (AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(PC->GetPawn())) {
		Player->SetMouseSensitivity(SaveData->MouseSensitivity);
	}

    // 동기화가 필요한 모든 IMC 에셋 수집 (데이터 파편화 방지)
	TArray<UInputMappingContext*> ContextsToUpdate;
	if (PC->GetSystemIMC()) ContextsToUpdate.Add(PC->GetSystemIMC());
	if (PC->GetDefaultIMC()) ContextsToUpdate.Add(PC->GetDefaultIMC());
	if (PC->GetDownedIMC()) ContextsToUpdate.Add(PC->GetDownedIMC());

    // 세이브 데이터 기반의 전수 조사 및 일괄 갱신
	for (auto& KVP : SaveData->KeyBindings) {
		FName TargetMappingName = KVP.Key;
		FKey NewKey = KVP.Value;

		// 등록된 모든 IMC를 순회하며 동기화
		for (UInputMappingContext* IMC : ContextsToUpdate) {
			
			// Action Name이 일치하는 매핑 탐색
			for (const FEnhancedActionKeyMapping& Mapping : IMC->GetMappings()) {
				bool bIsMatch = false;

                // Mappable Key Settings의 존재 여부에 따른 유연한 이름 비교
				if (UPlayerMappableKeySettings* MapSettings = Mapping.GetPlayerMappableKeySettings()) {
					bIsMatch = (MapSettings->Name == TargetMappingName);
				}
				else {
					bIsMatch = (Mapping.PlayerMappableOptions.Name == TargetMappingName);
				}

				if (bIsMatch) {
					// const_cast를 활용하여 읽기 전용(ReadOnly) 에셋의 데이터 보호를 해제하고 런타임 갱신
					const_cast<FEnhancedActionKeyMapping&>(Mapping).Key = NewKey;
					break;
				}
			}
		}

        // 입력 서브시스템 플러시 (Flush)
        // 메모리 상의 데이터는 변경되었으나, 서브시스템의 캐시를 갱신하기 위해 컨텍스트를 재등록함
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			AUnrealProjectCharacter* Player = Cast<AUnrealProjectCharacter>(PC->GetPawn());

            // 시스템 공통 조작계 갱신
			if (PC->GetSystemIMC()) {
				Subsystem->RemoveMappingContext(PC->GetSystemIMC());
				Subsystem->AddMappingContext(PC->GetSystemIMC(), 1);
			}

            // 캐릭터 상태(Downed 유무)에 따른 상황별 조작계 갱신
			if (Player && Player->IsDowned()) {
				Subsystem->RemoveMappingContext(PC->GetDownedIMC());
				Subsystem->AddMappingContext(PC->GetDownedIMC(), 0);
			}
			else {
				Subsystem->RemoveMappingContext(PC->GetDefaultIMC());
				Subsystem->AddMappingContext(PC->GetDefaultIMC(), 0);
			}
		}
	}
}
```

</div>
</details>  
</br>

### 3. 적의 넉백 무시
- 문제 상황 (Problem): 적에게 넉백 공격을 가했을 때, 적이 타격 방향으로 자연스럽게 밀려나지 않고 제자리에서 움찔거리거나 오히려 플레이어를 향해 미끄러지듯이 다가오는 물리 연산 오류가 발생했습니다.  

- 원인 분석 (Cause): 언리얼의 CharacterMovementComponent가 NavMesh를 따라 이동할 때 AI의 가속도와 지면 마찰력이 강하게 작용합니다. AI가 목표를 향해 나아가려는 내부 내비게이션 제어권 벡터와 넉백으로 뒤로 밀려나야 하는 외부 물리력(Impulse) 벡터가 동일 프레임에 충돌하면서 힘이 비정상적으로 상쇄되거나 왜곡되는 것이 원인이었습니다.  

- 해결 방법(Solution): 외부 물리력(Impulse)을 가하기 직전, StopMovementImmediately()를 호출하여 AI의 내비게이션 제어권과 기존 이동 관성을 0으로 명시적 초기화한 뒤 넉백을 적용했습니다. 물리 연산이 진행되는 동안 잠시 AI의 논리적 제어권을 엔진의 물리(Physics) 시스템에 완전히 양보하도록 설계하여, 적의 넉백을 안정적으로 구현할 수 있었습니다.   

<details>
<summary>적의 피격 및 넉백 함수</summary>
<div markdown="1">

```C++
float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	GetCharacterMovement()->StopMovementImmediately();

	// ... (생략) ...
}

void ABaseEnemy::HandleKnockback(FVector PushDirection, float Force)
{
	if (CurrentState == EEnemyState::EES_Dead) return;

	FVector FinalKnockback = PushDirection * Force;
	
	// 수직 부양력 보정
	float VerticalLiftMultiplier = 0.5f;
	FinalKnockback.Z = Force * VerticalLiftMultiplier;

	// 마찰력 씹힘 방지 및 공중 제어 불가
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->AirControl = 0.0f;

	this->LaunchCharacter(FinalKnockback, true, true);
}
```

</div>
</details>  
</br>

### 4. 3D 벡터 보정 및 애니메이션 예외 처리를 통한 피격 시스템 안정화
- 문제 상황 (Problem): 
    1. 방향 판별 왜곡: 피격 부위의 Z축(높이) 편차로 인해 정면에서 공격했음에도 측면 피격 모션이 재생되는 등 리액션 방향이 부정확하게 산출되는 현상이 발생했습니다.

    2. AI 영구 정지(Freeze): 특정 부위에 피격 애니메이션이 할당되지 않은 경우, 피격 상태(Hit State)에 진입한 적 AI가 다음 상태로 전이되지 못하고 영구적으로 멈춰버리는 결함이 발견되었습니다.

- 원인 분석 (Cause): 
    1. 벡터 연산 오차: 타격 위치(ImpactPoint)와 적 위치(ActorLocation)의 차 벡터를 계산할 때 Z축 성분이 포함되어, 내적(Dot) 및 외적(Cross)을 통한 2D 평면 각도 판별 시 3D 공간의 기울기가 오차로 작용했습니다.  

    2. 상태 머신 고착(State Lock): 애니메이션 몽타주 종료 통지를 기점으로 상태를 복구하는 구조에서, 유효하지 않은 몽타주가 실행될 경우 '종료 이벤트'가 발생하지 않아 FSM(상태 머신)이 피격 상태에 고착되는 것이 원인이었습니다.

- 해결 방법 (Solution): 
    1. 수학적 투영(Projection): 방향 연산 직전 ImpactPoint.Z를 타겟의 ActorLocation.Z와 동기화하여 순수 수평 2D(XY 평면) 벡터만 추출하도록 수식을 보정했습니다. 이를 통해 지형의 높낮이와 상관없이 정밀한 방향 판별을 보장했습니다.

    2. 예외 처리 및 Fallback 로직: 몽타주 재생 전 유효성 검사(IsValid)를 강화하고, 애니메이션 에셋이 없는 경우에도 즉시 AI 상태를 복구(Idle/Chase)하도록 예외 처리 루틴을 설계하여 시스템 안정성을 확보했습니다.

<details>
<summary>적의 피격 리액션 함수</summary>
<div markdown="1">

```C++
void ABaseEnemy::PlayDirectionalHitReact(const FVector& ImpactPoint)
{
	// 적의 정면 벡터
	FVector Forward = GetActorForwardVector();

	// 피격 지점 벡터(적의 위치 -> 때린 위치)
	/*FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z);
	FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();*/

	FVector EnemyLoc = GetActorLocation();
	FVector ToHit = (ImpactPoint - EnemyLoc);
	ToHit.Z = 0.f;  // Z축 간섭 제거
	ToHit = ToHit.GetSafeNormal();

	// 각도 구하기
	// 0도: 정면, 180/-180도: 후면, 90도: 우측, -90도: 좌측
	double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0) {
		Theta *= -1.0f;
	}

	UAnimMontage* MontageToPlay = nullptr;

	if (Theta >= -70.0f && Theta <= 70.0f) {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Front;
		UE_LOG(LogTemp, Warning, TEXT("Front Hit"));
	}
	else if (Theta >= -110.0f && Theta < -70.0f) {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Left;
		UE_LOG(LogTemp, Warning, TEXT("Left Hit"));
	}
	else if (Theta > 70.0f && Theta <= 110.0f) {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Right;
		UE_LOG(LogTemp, Warning, TEXT("Right Hit"));
	}
	else {
		MontageToPlay = CurrentEnemyStat.HitReactMontage_Back;
		UE_LOG(LogTemp, Warning, TEXT("Back Hit"));
	}

	// 몽타주 재생 시도 
	float Duration = 0.0f;

	if (MontageToPlay)
	{
		Duration = PlayAnimMontage(MontageToPlay);
	}

    // Duration이 0이라면 애니메이션이 없는 것으로 판별
	if (Duration <= 0.0f)
	{
		// 임시로 0.5초 동안 몸이 굳은(Stun) 상태를 유지하다가 땅에 떨어지면 풀리도록 세팅
		FTimerHandle FakeHitReactTimer;
		GetWorldTimerManager().SetTimer(FakeHitReactTimer, FTimerDelegate::CreateLambda([this]()
			{
				if (!GetCharacterMovement()->IsFalling())
				{
					CurrentState = EEnemyState::EES_Normal;
					if (AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController()))
					{
						if (UBrainComponent* Brain = AIController->GetBrainComponent())
						{
							Brain->ResumeLogic("Hit Reaction");
						}
					}
				}
			}), 0.5f, false);
	}
}
```

</div>
</details>  
</br>

## 회고
### 1. 엔진의 사용자를 넘어, 시스템을 설계하는 개발자로
이전 프로젝트인 C++ WinAPI 기반 프레임워크 제작에서 고민했던 '컴포넌트 패턴'과 '메모리 관리 경험은, 언리얼 엔진 5를 다룰 때 길을 잃지 않게 해 준 나침반 역할을 해주었습니다. 단순히 엔진의 블루프린트 기능을 사용하는 것이 아닌 ActorComponent로 책임을 분리하며 GameMode와 GameState의 생명주기를 활용해 아키텍처를 통제했습니다. 이를 통해 단단한 객체 지향 설계(OOP)가 프로젝트의 확장성과 퍼포먼스를 결정한다는 확신을 얻을 수 있었습니다.  

### 2. 눈에 보이지 않는 안전함을 코딩한다
개발 과정은 수많은 크래시(Crash)와 메모리 누수와의 싸움이었습니다. 비동기 타이머 로직과 델리게이트(Delegate)가 얽히며 발생하는 댕글링 포인트(Dagling Pointer), 읽기 전용 에셋(IMC)의 런타임 동기화 실패 등 엔진의 로우레벨 작동 방식을 모르면 해결할 수 없는 난관에 부딪혔습니다. 그러나 문제를 적당히 우회하지 않고, const_cast를 통한 제한적 메모리 권한 획득, 명시적인 생명주기(Lifecycle) 해제, 3D 벡터 수학(Math)을 이용한 연산 보정 등 근본적인 구조와 수학적 접근으로 문제를 해결하였습니다. 이 과정을 겨치며 예외 상황을 해결하는 견고한 시스템을 구축하는 디버깅 역량을 기를 수 있었습니다.  

### 3. 협업을 준비하는 1인 개발자
비록 1인 개발로 진행된 프로젝트이지만, 기획 데이터(DataTable)와 핵심 C++ 로직을 완벽히 분리한 데이터 주도(Data-Driven) 파이프라인을 구축하여 언제는 기획자나 다른 프로그래머가 합류해도 즉각적인 협업이 가능하도록 환경을 설계했습니다.  

## 마치며
이번 언리얼 프로젝트는 수십 마리의 오브젝트를 어떻게 효율적으로 렌더링하고 관리할 것인가에 대한 제 치열한 고민의 결과물입니다. 앞으로 게임 산업 현장에서 마주하게 될 더 거대한 규모의 프로젝트와 복잡한 최적화 이슈 앞에서도, 흔들리지 않는 기본기와 문제 해결 능력을 바탕으로 팀의 비전을 현실로 구현해 내는 신뢰할 수 있는 게임 프로그래머가 되겠습니다.  
