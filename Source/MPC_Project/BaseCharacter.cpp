// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h" // 캡슐 반경 가져오기용 필수
#include "AIController.h"
#include "DiceManager.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/World.h"
#include "TurnManagerSubsystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "ActionEconomyComponent.h"
#include "CombatComponent.h"
#include "EquipmentComponent.h"
#include "BuffComponent.h"
#include "AnimationComponent.h"
#include "InteractComponent.h"
#include "InventoryComponent.h"
#include "FieldItemActor.h"
#include "UnitRegistrySubsystem.h"
#include "FlowFieldSubsystem.h"
#include "SquadManager.h"
#include "AIController.h" 

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 선택 표시용 데칼 세팅 (원하는 경우)
	SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
	SelectionDecal->SetupAttachment(RootComponent);
	SelectionDecal->SetVisibility(false); // 처음엔 숨김

	TempMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempMesh"));
	TempMesh->SetupAttachment(RootComponent);

	// 행동경제 컴포넌트 생성!
	ActionEconomyComponent = CreateDefaultSubobject<UActionEconomyComponent>(TEXT("ActionEconomyComponent"));

	//	CombatComponent 생성!
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// EquipmentComponent 생성
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));

	//	BuffComponent 생성
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));

	// AnimationComponent 생성 
	AnimationComponent = CreateDefaultSubobject<UAnimationComponent>(TEXT("AnimationComponent"));

	// AbilityComponent 생성
	AbilityComp = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComp"));

	// InteractComponent 생성
	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));

	//	InventoryComponent 생성
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// 1. 기본 AIController 클래스 지정 (필요시 커스텀 AIController 클래스로 변경 가능)
	AIControllerClass = AAIController::StaticClass();

	// 2. 월드에 배치되거나 SpawnActor로 동적 생성될 때 모두 AIController 자동 생성/빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 위치 저장
	PreviousFrameLocation = GetActorLocation();

	// 게임 시작 시 소켓에 꽂혀 있는 데이터들로 스탯 및 무기 초기화
	//InitializeCharacterFromDataSockets();
 
	// 월드 서브시스템에 자신을 등록!
	if (UUnitRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UUnitRegistrySubsystem>())
	{
		Registry->RegisterUnit(this);
	}
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 사망/삭제 시 서브시스템에서 자신을 등록 해제!
	if (UUnitRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UUnitRegistrySubsystem>())
	{
		Registry->UnregisterUnit(this);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	// 1. 턴 모드 실시간 이동거리 차감 (Flow Field 및 수동 이동 공통)
	if (bInCombat && bIsMyTurn && ActionEconomyComponent)
	{
		const FVector CurrentLocation = GetActorLocation();
		const float MovedDistance = FVector::Distance(CurrentLocation, PreviousFrameLocation);

		if (MovedDistance > 0.0f)
		{
			if (!ActionEconomyComponent->ConsumeMoveDistance(MovedDistance))
			{
				StopMovement();
				bIsManualMoving = false;
				StopFlowFieldMovement(); // 이동 자원 소진 시 정지
			}
		}
	}

	PreviousFrameLocation = GetActorLocation();

	if (bInCombat && !bIsMyTurn) return;

	// 2. Flow Field 기반 이동 처리
	if (bIsMovingWithFlowField && MySquadManager)
	{
		FVector MyLocation = GetActorLocation();
		FVector MyFormationTarget = MySquadManager->MemberTargetLocations.Contains(this) ? MySquadManager->MemberTargetLocations[this] : MyLocation;
		FVector MainTarget = MySquadManager->CurrentMainTargetLocation;

		// 1. 부대 중심(MainTarget) 및 내 지정석(MyFormationTarget)과의 거리 모두 계산
		float DistanceToMainTarget = FVector::Dist2D(MyLocation, MainTarget);
		float DistanceToMyTarget = FVector::Dist2D(MyLocation, MyFormationTarget);

		// 2. 부대가 목적지 반경에 들어왔거나, '내'가 '내 지정석' 반경에 들어오면 로컬 진형 모드 발동
		// (대형이 넓게 퍼지는 것을 고려하여 반경을 400 -> 1000.0f 로 넉넉하게 늘립니다)
		if (DistanceToMainTarget < 1000.0f || DistanceToMyTarget < 1000.0f)
		{
			// [근거리 모드] FlowField를 무시하고 자신의 고유 진형 위치로 직접 이동
			FVector DirectionToFormation = (MyFormationTarget - MyLocation);
			DirectionToFormation.Z = 0.0f; // Z축(높이) 차이 무시

			// [수정] 물리 캡슐 크기를 고려하여 도착 판정 여유 범위를 50.0f로 넓힘
			if (DirectionToFormation.Size2D() < 50.0f)
			{
				StopFlowFieldMovement();
			}
			else
			{
				AddMovementInput(DirectionToFormation.GetSafeNormal(), 1.0f);
			}
		}
		else
		{
			// [원거리 모드] 거리가 멀 때는 FlowField 방향 화살표를 따라 이동
			FVector MoveDirection = MySquadManager->GetMoveDirectionForUnit(MyLocation);

			if (MoveDirection.IsNearlyZero())
			{
				StopFlowFieldMovement();
			}
			else
			{
				AddMovementInput(MoveDirection, 1.0f);
			}
		}
	}
	else if (bIsMovingWithFlowField && !MySquadManager)
	{
		StopFlowFieldMovement();
	}

	// 3. 기존 수동 이동 완료 체크
	if (bIsManualMoving)
	{
		if (HasReachedManualDestination())
		{
			bIsManualMoving = false;
		}
		return;
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}

void ABaseCharacter::SetSelected(bool bSelected)
{
	// 1. 내 부대장(SquadManager)이 존재한다면, 개별 선택 대신 부대 전체 선택으로 위임!
	//if (MySquadManager)
	//{
	//	// 부대장이 전체 부대원들의 bIsSelected와 데칼을 일괄 통제하도록 위임
	//	MySquadManager->SetSelected(bSelected);
	//	return;
	//}

	//	무한 스택!!!!

	// 2. 부대가 없는 솔로 유닛일 경우 기존 단일 선택 로직 수행
	bIsSelected = bSelected;

	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bIsSelected);
	}

	if (bIsSelected)
	{
		UE_LOG(LogTemp, Log, TEXT("유닛이 선택되었습니다: %s"), *GetName());
	}

}

void ABaseCharacter::MoveToLocation(const FVector& TargetLocation)
{
	// 자기 자신의 Controller를 얻어 스스로 NavMesh 이동 실행
	if (AController* MyController = GetController())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, TargetLocation);
	}
}

void ABaseCharacter::StopMovement()
{
	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
	}
}

void ABaseCharacter::ApplyBuffToSelf(FDataTableRowHandle BuffRow)
{
		if (BuffComponent)
		{
			BuffComponent->ApplyBuff(BuffRow);
		}
}

void ABaseCharacter::CommandTargetAction(AActor* InTargetActor, EActionTargetType TargetType)
{

	// [디버그 로그] 우클릭 신호가 여기까지 들어오는지 확인!
	UE_LOG(LogTemp, Warning, TEXT("[%s] CommandTargetAction 들어옴! Target: %s / TargetType: %d"),
		*GetName(), InTargetActor ? *InTargetActor->GetName() : TEXT("Null"), (int32)TargetType);

	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	if (!InTargetActor) return;

	// 1. 적(Enemy) 대상 공격 명령
	if (TargetType == EActionTargetType::Enemy)
	{
		// 기존 상호작용 추적 중단
		if (InteractComponent)
		{
			InteractComponent->ClearInteractTarget();
		}

		// CombatComponent에 공격 명령 요청
		if (CombatComponent && CombatComponent->RequestAttackCommand(InTargetActor, bInCombat, bIsMyTurn))
		{
			CurrentCommand = TargetType;
			bIsManualMoving = false;
		}
		return;
	}

	// 2. 상호작용(Interactable) 및 루팅(Loot) 대상 명령 
	if (TargetType == EActionTargetType::Interactable || TargetType == EActionTargetType::Loot)
	{
		// 기존 전투 공격 추적 중단
		if (CombatComponent)
		{
			CombatComponent->ClearCombatTarget();
		}

		// InteractComponent에 상호작용 명령 요청 (사거리 추적 이동 및 Skill Check 집행)
		if (InteractComponent)
		{
			InteractComponent->RequestInteractCommand(InTargetActor);
			CurrentCommand = TargetType;
			bIsManualMoving = false;
			UE_LOG(LogTemp, Log, TEXT("[%s] 상호작용 명령 수신: %s"), *GetName(), *InTargetActor->GetName());
		}
		return;
	}
}

void ABaseCharacter::CommandTargetAction(EActionTargetType TargetType, const FVector& TargetLocation)
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	if (bInCombat && !bIsMyTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Not my turn! Ground Move Ignored."), *GetName());
		return;
	}

	if (TargetType == EActionTargetType::Ground)
	{
		// 턴 모드 잔여 이동거리 검사
		if (bInCombat && ActionEconomyComponent)
		{
			if (ActionEconomyComponent->GetRemainingMoveDistance() < 50.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Move Command Rejected: Insufficient remaining move distance!"), *GetName());
				return;
			}
		}

		// 이미 지면 이동 중이라면 ClearCommand()를 부르지 않음!
		if (!bIsManualMoving)
		{
			ClearCommand(); // 적 공격 등 기존 타깃 명령이 있었을 때만 1회 정리[cite: 10]
		}

		bIsManualMoving = true;

		// 목적지가 기존 위치와 크게 차이가 있을 때만 NavMesh 이동 수행 (30cm 임계값)
		if (FVector::DistSquared(GetTargetLocation(), TargetLocation) > 900.0f) // 30^2 = 900
		{
			MoveToLocation(TargetLocation);
		}
	}
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

void ABaseCharacter::ApplyDamage(int32 DamageAmount)
{
	CharacterData.CurrentHP -= DamageAmount;
	CharacterData.CurrentHP = FMath::Max(0, CharacterData.CurrentHP);

	UE_LOG(LogTemp, Warning, TEXT("[%s] Current HP: %d / %d"),
		*GetName(), CharacterData.CurrentHP, CharacterData.MaxHP);

	if (CharacterData.CurrentHP <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Has Died!"), *GetName());

		//	사망 시 즉시 모든 명령 및 이동 정리!
		ClearCommand();
		StopMovement();
	}
}

int32 ABaseCharacter::GetAttackBonus(EWeaponRangeType AttackType) const
{
	int32 BAB = CharacterData.BaseAttackBonus;
	int32 STRMod = CharacterData.GetSTRBonus();
	int32 DEXMod = CharacterData.GetDEXBonus();
	int32 FinalBonus = BAB;

	if (AttackType == EWeaponRangeType::Melee)
	{
		if (EquipmentComponent && EquipmentComponent->IsFinesseWeapon())
		{
			FinalBonus += FMath::Max(STRMod, DEXMod);
		}
		else
		{
			FinalBonus += STRMod;
		}
	}
	else if (AttackType == EWeaponRangeType::Ranged)
	{
		FinalBonus += DEXMod;
	}
	else
	{
		FinalBonus += STRMod;
	}

	// 버프 명중 보너스 합산 추가!
	if (BuffComponent)
	{
		FinalBonus += BuffComponent->GetTotalAttackRollBonus();
	}

	return FinalBonus;
}

int32 ABaseCharacter::GetDamageBonus() const
{
	int32 STRMod = CharacterData.GetSTRBonus();

	// 양손 무기 체크!
	if (EquipmentComponent && EquipmentComponent->IsTwoHandedWeapon())
	{
		return FMath::FloorToInt(STRMod * 1.5f);
	}

	return STRMod;
}

int32 ABaseCharacter::GetSavingThrowBonus(ESavingThrowType SaveType) const
{
	int32 StatBonus = 0;

	switch (SaveType)
	{
	case ESavingThrowType::Fortitude:
		StatBonus = CharacterData.GetCONBonus(); // 체질 수정치
		break;
	case ESavingThrowType::Reflex:
		StatBonus = CharacterData.GetDEXBonus(); // 민첩 수정치
		break;
	case ESavingThrowType::Will:
		StatBonus = CharacterData.GetWISBonus(); // 지혜 수정치
		break;
	default:
		break;
	}

	// 버프 컴포넌트가 있다면 버프 보너스도 합산
	if (BuffComponent)
	{
		// 필요 시 BuffComponent->GetTotalSaveBonus(SaveType) 추가 연동 가능
	}

	return StatBonus;
}

bool ABaseCharacter::RequestUseItemAtIndex(int32 SlotIndex, int32 Quantity)
{
	if (!InventoryComponent || !AbilityComp) return false;

	// 1. 슬롯 데이터 검사
	const FInventorySlotInfo* SlotInfo = InventoryComponent->GetSlotInfoAtIndex(SlotIndex);
	if (!SlotInfo || SlotInfo->IsEmpty()) return false;

	const FItemMasterDataRow& ItemData = SlotInfo->ItemData;

	// 2. 소비품(Consumable) 및 두루마리(Scroll) 처리
	if (ItemData.ItemType == EItemType::Consumable || ItemData.ItemType == EItemType::Scroll)
	{
		// 연결된 스킬(LinkedSkillRow)이 있는지 확인
		if (ItemData.LinkedSkillRow.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Item '%s' has NO LinkedSkillRow assigned in DT_Item!"),
				*GetName(), *ItemData.ItemName.ToString());
			return false;
		}

		// 턴제 전투 중 행동 자원 검사
		UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
		if (TurnSys && TurnSys->IsCombatActive())
		{
			if (!bIsMyTurn || !ActionEconomyComponent || !ActionEconomyComponent->HasStandardAction())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot use item: Standard Action required during combat!"), *GetName());
				return false;
			}
		}

		// 3. AbilityComponent에 아이템 연결 스킬 주입 (시전 준비)
		SetPendingSkill(ItemData.LinkedSkillRow);

		if (HasPendingSkill())
		{
			// 타깃이 '자기 자신(Self)'일 경우 즉시 시전 실행
			// (스킬 내부에 설정된 주사위 굴림, HP 회복, 파티클 재생, 내성 굴림 등이 자동으로 집행됨)
			bool bExecuted = ExecutePendingSkill(this, GetActorLocation());

			if (bExecuted)
			{
				// 턴제 전투 중이면 표준 행동 소모
				if (TurnSys && TurnSys->IsCombatActive() && ActionEconomyComponent)
				{
					ActionEconomyComponent->ConsumeStandardAction();
				}

				UE_LOG(LogTemp, Warning, TEXT("[%s] Successfully used item '%s' via Linked Skill!"),
					*GetName(), *ItemData.ItemName.ToString());

				// 시전 성공시에만 인벤토리 수량 1개 차감
				return InventoryComponent->RemoveItemAtIndex(SlotIndex, Quantity);
			}
			else
			{
				// 시전 실패 시 (만피 상태이거나 대상 불능 등) 스킬 조준 취소
				ClearPendingSkill();
				UE_LOG(LogTemp, Warning, TEXT("[%s] Skill execution failed for item '%s'."),
					*GetName(), *ItemData.ItemName.ToString());
			}
		}
	}

	return false;
}

bool ABaseCharacter::TryAddItemToInventory(const FItemMasterDataRow& ItemData, int32 Quantity)
{
	if (InventoryComponent)
	{
		return InventoryComponent->AddItem(ItemData, Quantity);
	}
	return false;
}

bool ABaseCharacter::RequestSwapSlots(int32 FromIndex, int32 ToIndex)
{
	if (InventoryComponent)
	{
		return InventoryComponent->SwapSlots(FromIndex, ToIndex);
	}
	return false;
}

bool ABaseCharacter::RequestDropItemToWorld(int32 SlotIndex, int32 Quantity)
{
	if (!InventoryComponent) return false;

	const FInventorySlotInfo* SlotInfo = InventoryComponent->GetSlotInfoAtIndex(SlotIndex);
	if (!SlotInfo || SlotInfo->IsEmpty()) return false;

	// 1. 드랍할 아이템 복사
	FItemMasterDataRow DropData = SlotInfo->ItemData;
	int32 DropQty = FMath::Min(SlotInfo->Quantity, Quantity);

	// 2. 인벤토리에서 수량 차감
	if (InventoryComponent->RemoveItemAtIndex(SlotIndex, DropQty))
	{
		// 3. 캐릭터 앞 지면에 AFieldItemActor 스폰
		FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 100.0f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		AFieldItemActor* DroppedActor = GetWorld()->SpawnActor<AFieldItemActor>(AFieldItemActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (DroppedActor)
		{
			DroppedActor->InitializeItemFromData(DropData, DropQty);
			return true;
		}
	}
	return false;
}


bool ABaseCharacter::HasReachedManualDestination() const
{
	// 1. 캐릭터의 현재 이동 속도가 거의 0인지 체크 (멈췄는지 여부)
	bool bIsStopped = GetVelocity().SizeSquared() < 100.0f; // 속도의 제곱값 비교 (약 10cm/s 이하)

	// 2. AIController를 통해 이동 경로 상태 확인
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon)
	{
		// 경로 이동 상태가 이동 중(Following)이 아닌 경우
		EPathFollowingStatus::Type PathStatus = AICon->GetMoveStatus();
		bool bPathFinished = (PathStatus != EPathFollowingStatus::Moving);

		return bIsStopped && bPathFinished;
	}
	
	return bIsStopped;
}

bool ABaseCharacter::CanPerformAction() const
{
	// 대기/이동(IdleRun) 상태일 때만 새로운 이동이나 공격 명령을 수락
	return GetCurrentAnimState() == ECharacterAnimState::IdleRun;
}
void ABaseCharacter::StartFlowFieldMovement()
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	// 1. 내 턴이 아니면 이동 명령 차단
	if (bInCombat && !bIsMyTurn) return;

	// 2. 턴 모드일 때 이동 잔여 자원 검사는 캐릭터 스스로 내부 컴포넌트를 확인!
	if (bInCombat && ActionEconomyComponent)
	{
		if (ActionEconomyComponent->GetRemainingMoveDistance() < 50.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Move Rejected: Insufficient distance!"), *GetName());
			return;
		}
	}

	// 3. 기존 NavMesh 이동, 공격, 상호작용 등 다른 명령 정리
	ClearCommand();

	// 4. Flow Field 이동 플래그 활성화
	bIsMovingWithFlowField = true;
}
void ABaseCharacter::StopFlowFieldMovement()
{
	bIsMovingWithFlowField = false;
	StopMovement(); // CharacterMovementComponent의 이동 가속도 초기화
}
ISelectableInterface* ABaseCharacter::GetSelectionOwner()
{
	// 내 부대장(MySquadManager)이 존재한다면 부대장의 ISelectableInterface 반환!
	if (MySquadManager && MySquadManager->Implements<USelectableInterface>())
	{
		return Cast<ISelectableInterface>(MySquadManager);
	}
	return this;
}
void ABaseCharacter::SetPendingSkill(const FDataTableRowHandle& SkillRow)
{
	if (AbilityComp) AbilityComp->SetPendingSkill(SkillRow);
}
void ABaseCharacter::ClearPendingSkill()
{
	if (AbilityComp) AbilityComp->ClearPendingSkill();
	UE_LOG(LogTemp, Warning, TEXT("[BP_BaseCharacter] Spell Stance Cancelled by %s"), ANSI_TO_TCHAR(__FUNCTION__));
}
bool ABaseCharacter::HasPendingSkill() const
{
	return AbilityComp ? AbilityComp->HasPendingSkill() : false;
}
bool ABaseCharacter::ExecutePendingSkill(AActor* InTargetActor, const FVector& TargetLocation)
{
	if (!AbilityComp) return false;

	// 애니메이션 상태만 전달 후 실제 실행 및 검증은 AbilityComp가 전담!
	SetCurrentAnimState(ECharacterAnimState::CastingSpell);

	bool bSuccess = AbilityComp->ExecutePendingSkill(InTargetActor, TargetLocation);

	if (!bSuccess)
	{
		SetCurrentAnimState(ECharacterAnimState::IdleRun);
	}

	return bSuccess;
}
void ABaseCharacter::SetPendingSkillBySlot(int32 SlotIndex)
{
	if (AbilityComp) AbilityComp->SetPendingSkillBySlot(SlotIndex);
}
bool ABaseCharacter::GetSkillDataBySlotByCharacter(int32 SlotIndex, FSkillDataRow& OutSkillData) const
{
	if (AbilityComp)
	{
		return AbilityComp->GetSkillDataBySlot(SlotIndex, OutSkillData);
	}
	return false;
}
ECharacterAnimState ABaseCharacter::GetCurrentAnimState() const
{
	if (AnimationComponent)
	{
		return AnimationComponent->GetAnimState();
	}
	return ECharacterAnimState::IdleRun;
}
void ABaseCharacter::SetCurrentAnimState(ECharacterAnimState NewState)
{
	if (AnimationComponent)
	{
		AnimationComponent->SetAnimState(NewState);
	}
}

float ABaseCharacter::PlayAnimationMontage(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName)
{
	if (AnimationComponent)
	{
		return AnimationComponent->PlayMontage(MontageToPlay, InPlayRate, StartSectionName);
	}

	// 예외 처리: 혹시 AnimationComponent가 없을 때만 ACharacter 기본 함수 호출
	return PlayAnimMontage(MontageToPlay, InPlayRate, StartSectionName);
}

void ABaseCharacter::ApplyDamageAmount(int32 Damage, AActor* DamageCauser)
{
	CharacterData.CurrentHP -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Took %d Damage! Remaining HP: %d"),
		*GetName(), Damage, CharacterData.CurrentHP);

	//  HP 변경 방송!
	OnHealthChanged.Broadcast(CharacterData.CurrentHP, CharacterData.MaxHP);

	if (CharacterData.CurrentHP <= 0)
	{
		// 사망 처리 로직 (필요시 추가)
		UE_LOG(LogTemp, Error, TEXT("[%s] Has Died!"), *GetName());
	}
}

void ABaseCharacter::ApplyHealingAmount(int32 HealAmount, AActor* Healer)
{
	if (HealAmount <= 0) return;

	const int32 OldHP = CharacterData.CurrentHP;

	// Clamp 처리로 Overheal 방지
	CharacterData.CurrentHP = FMath::Clamp(CharacterData.CurrentHP + HealAmount, 0, CharacterData.MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("[%s] Healed %d HP! (%d -> %d / Max: %d)"),
		*GetName(), HealAmount, OldHP, CharacterData.CurrentHP, CharacterData.MaxHP);

	// 체력 변경 UI 및 HUD 알림 방송
	OnHealthChanged.Broadcast(CharacterData.CurrentHP, CharacterData.MaxHP);
}

int32 ABaseCharacter::GetCurrentHP() const
{
	return CharacterData.CurrentHP;
}

int32 ABaseCharacter::GetArmorClass() const
{
	int32 TotalAC = CharacterData.GetTotalAC();

	if (EquipmentComponent)
	{
		const FArmorDataRow& Armor = EquipmentComponent->GetCurrentArmorData();
		TotalAC = CharacterData.GetTotalAC(Armor.ArmorClassBonus, Armor.MaxDexBonus);
	}

	// ★ 버프 AC 보너스 합산 추가!
	if (BuffComponent)
	{
		TotalAC += BuffComponent->GetTotalACBonus();
	}

	return TotalAC;
}

void ABaseCharacter::ToggleHoldPosition()
{
	bIsHoldPosition = !bIsHoldPosition;

	if (bIsHoldPosition)
	{
		// bIsMovingToAttack = false; 제거!
		StopMovement();

		// 전투 컴포넌트가 추적 중이었다면 멈추도록 전달
		if (CombatComponent)
		{
			CombatComponent->StopAttackCommand();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] Hold Position: %s"), *GetName(), bIsHoldPosition ? TEXT("ON") : TEXT("OFF"));
}

void ABaseCharacter::ToggleAutoCombat()
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();

	// 턴제 전투 진행 중이라면 자동 전투 토글 차단!
	if (TurnSys && TurnSys->IsCombatActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot toggle AutoCombat during turn-based combat!"), *GetName());
		return;
	}

	// 탐험 모드일 때만 토글 허용
	bAutoCombatEnabled = !bAutoCombatEnabled;

	// [로그 추가] 현재 변경된 자동 전투 상태 출력
	UE_LOG(LogTemp, Warning, TEXT("[%s] AutoCombat Toggled -> %s"),
		*GetName(), bAutoCombatEnabled ? TEXT("ON (True)") : TEXT("OFF (False)"));
}

void ABaseCharacter::SetTurnActive(bool bActive)
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();

	if (TurnSys && TurnSys->IsCombatActive())
	{
		if (bIsMyTurn != bActive && bActive)
		{
			bPreCombatAutoCombatEnabled = bAutoCombatEnabled;
		}
		bAutoCombatEnabled = false; // 전투 중엔 자동 전투 Off

		// 이전 지면 이동 정지
		StopMovement();
		bIsManualMoving = false;

		// 전투 컴포넌트 추적 및 공격 명령 정지
		if (CombatComponent)
		{
			CombatComponent->StopAttackCommand();
		}
	}

	bIsMyTurn = bActive;

	if (bActive)
	{
		OnTurnStarted();
	}
	else
	{
		OnTurnEnded();
	}
}

bool ABaseCharacter::IsMyTurn() const
{
	return bIsMyTurn;
}

int32 ABaseCharacter::GetDexterityModifier() const
{
	// D&D/Pathfinder 룰: (스탯 - 10) / 2
	return CharacterData.GetDEXBonus();
}
void ABaseCharacter::ClearCommand()
{
	CurrentCommand = EActionTargetType::None;
	bIsManualMoving = false;
	bIsMovingWithFlowField = false; // 명령 청소 시 Flow Field 이동도 깨끗하게 정지

	StopMovement();

	if (CombatComponent) CombatComponent->ClearCombatTarget();
	if (InteractComponent) InteractComponent->ClearInteractTarget();
}

void ABaseCharacter::OnTurnStarted()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] OnTurnStarted called!"), *GetName());

	StopMovement();
	bIsManualMoving = false;

	if (CombatComponent)
	{
		CombatComponent->StopAttackCommand();
	}

	if (ActionEconomyComponent)
	{
		ActionEconomyComponent->ResetActionEconomy();
	}
}

void ABaseCharacter::OnTurnEnded()
{
	bIsMyTurn = false;
	ClearCommand(); // 턴이 끝나면 이동 및 공격 명령 전체 해제
	UE_LOG(LogTemp, Warning, TEXT("[%s] OnTurnEnded called!"), *GetName());
}

bool ABaseCharacter::HasRemainingActions() const
{
	//	컴포넌트에 자원이 남아있는지 검사
	if (!ActionEconomyComponent) return false;

	// 표준 행동이 남아있거나, 잔여 이동거리가 남아있으면 행동 가능 상태
	return ActionEconomyComponent->HasStandardAction() || (ActionEconomyComponent->GetRemainingMoveDistance() > 0.0f);
}

void ABaseCharacter::ResetToExplorationState()
{
	bIsMyTurn = true; // 탐험 모드에선 자유 이동
	bAutoCombatEnabled = bPreCombatAutoCombatEnabled; // 백업해둔 탐험 모드 상태 복구!
	ClearCommand();
}
