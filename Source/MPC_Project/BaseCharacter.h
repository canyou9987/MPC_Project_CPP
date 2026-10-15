// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SelectableInterface.h"
#include "DamageableInterface.h"
#include "CombatStatInterface.h"
#include "TurnParticipantInterface.h"
#include "BaseCharData.h"
#include "CRPGDataType.h"
#include "MovableInterface.h"
#include "BuffableInterface.h"
#include "AnimStateInterface.h"
#include "FactionInterface.h"
#include "AbilityComponent.h"
#include "SkillCasterInterface.h" 
#include "InventoryHolderInterface.h"
#include "BaseCharacter.generated.h"	//	항상 최하단일것

// 전방 선언 (Forward Declaration)
class UActionEconomyComponent;
class UCombatComponent;		// CombatComponent 추가
class UEquipmentComponent;	//	EquipmentComponent 추가
class UBuffComponent;
class AnimationComponent;
class UInteractComponent;
class UInventoryComponent;
class ASquadManager;

UCLASS()
class MPC_PROJECT_API ABaseCharacter : public ACharacter ,
	public ISelectableInterface , 
	public IDamageableInterface , 
	public ITurnParticipantInterface , 
	public ICombatStatInterface ,
	public IMovableInterface,
	public IBuffableInterface,
	public IAnimStateInterface,
	public IFactionInterface,
	public ISkillCasterInterface,
	public IInventoryHolderInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//	선택 및 이동 함수
	virtual void SetSelected(bool bSelected) override;
	virtual bool IsSelected() const override { return bIsSelected; }
	//virtual void MoveToLocation(const FVector& TargetLocation) override;

	// CameraPawn에서 전달하는 통합 명령 인터페이스
	virtual void CommandTargetAction(AActor* InTargetActor, EActionTargetType TargetType) override;
	virtual void CommandTargetAction(EActionTargetType TargetType, const FVector& TargetLocation) override;
	virtual EActionTargetType GetActionTargetType() const override { return CharacterTargetType; }


	// 스탯 데이터 읽기용 Getter
	const FBaseCharData& GetCharacterData() const { return CharacterData; }

	// 언리얼 기본 데미지 수용 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	// 근접 명중 보너스 반환 함수
	int32 GetMeleeAttackBonus() const { return CharacterData.GetMeleeHitBonus(); }

	void ApplyDamage(int32 DamageAmount);

	// ICombatStatInterface 상속 함수
	virtual int32 GetAttackBonus(EWeaponRangeType AttackType = EWeaponRangeType::Melee) const override;
	virtual int32 GetDamageBonus() const override;
	virtual int32 GetSavingThrowBonus(ESavingThrowType SaveType) const override; // 내성굴림 받아오기!

	// IDamageableInterface 함수 오버라이딩
	virtual void ApplyDamageAmount(int32 Damage, AActor* DamageCauser) override;		//	상대한테 데미지 적용하기
	virtual void ApplyHealingAmount(int32 HealAmount, AActor* Healer) override;			//	상대한테 힐시키기(음수 데미지 포함)
	virtual int32 GetMaxHP() const override { return CharacterData.MaxHP; }				//	최대 체력받아오기
	virtual int32 GetCurrentHP() const override;										//  상대 체력 받아오기
	virtual int32 GetArmorClass() const override;//  AC 반환 연결!
	virtual FOnHealthChanged* GetOnHealthChangedDelegate() override { return &OnHealthChanged; }

	// ==========================================
	// ISelectableInterface 오버라이딩
	// ==========================================
	virtual void ToggleHoldPosition() override;
	virtual void ToggleAutoCombat() override;
	// Flow Field 이동 시작/중지 명령
	virtual void StartFlowFieldMovement() override;
	virtual void StopFlowFieldMovement() override;
	//	스퀘드 매니저 ID 겟터
	virtual ISelectableInterface* GetSelectionOwner() override;

	// ITurnParticipantInterface 오버라이딩 추가 ---
	virtual void SetTurnActive(bool bActive) override;			//	턴 셋팅함수
	virtual bool IsMyTurn() const override;						//	자기턴이 무엇인지 반환하는 bIsMyTurn를체크
	virtual int32 GetDexterityModifier() const override;		//	민첩 수정치를 반환
	virtual void ClearCommand() override;						//	명령 초기화
	virtual void OnTurnStarted() override;						//	턴 Intit
	virtual void OnTurnEnded() override;						//	턴 청소
	virtual bool HasRemainingActions() const override;			//	잔여 행동이 있는지 검사
	virtual void ResetToExplorationState() override;			//	탐험 모드로 전환

	// IMovableInterface 오버라이딩
	virtual void MoveToLocation(const FVector& TargetLocation) override;
	virtual void StopMovement() override;

	// IBuffableInterface 오버라이딩 구현!
	virtual void ApplyBuffToSelf(FDataTableRowHandle BuffRow) override;

	// IAnimStateInterface 구현
	virtual ECharacterAnimState GetCurrentAnimState() const override;
	virtual float GetCurrentMovementSpeed() const override { return GetVelocity().Size2D(); }
	virtual void SetCurrentAnimState(ECharacterAnimState NewState) override;			// 애니메이션 상태 Getter / Setter
	virtual float PlayAnimationMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.0f, FName StartSectionName = NAME_None) override;

	// IFactionInterface 구현
	virtual EFactionGroup GetFactionGroup() const override { return FactionGroup; }
	//void SetFaction(EFactionType NewFaction) { Faction = NewFaction; }

	// ==========================================
	// ISkillCasterInterface 오버라이딩
	// ==========================================
	virtual void SetPendingSkill(const FDataTableRowHandle& SkillRow) override;
	virtual void ClearPendingSkill() override;
	virtual bool HasPendingSkill() const override;
	virtual bool ExecutePendingSkill(AActor* InTargetActor, const FVector& TargetLocation) override;
	virtual void SetPendingSkillBySlot(int32 SlotIndex) override;
	virtual bool GetSkillDataBySlotByCharacter(int32 SlotIndex, FSkillDataRow& OutSkillData) const override;

	// ==========================================
	// IInventoryHolderInterface 오버라이딩
	// ==========================================

	//	UInventoryComponent 게터
	virtual UInventoryComponent* GetInventoryComponent() const override { return InventoryComponent; }

	virtual bool RequestUseItemAtIndex(int32 SlotIndex, int32 Quantity = 1) override;
	virtual bool TryAddItemToInventory(const FItemMasterDataRow& ItemData, int32 Quantity) override; // 필드 아이템을 추가하기위한 함수
	// 슬롯 스와프 및 월드 드롭 요청 함수
	virtual bool RequestSwapSlots(int32 FromIndex, int32 ToIndex) override;
	virtual bool RequestDropItemToWorld(int32 SlotIndex, int32 Quantity = 1) override;


	// 수동 이동 완료 여부를 확인하는 헬퍼 함수
	bool HasReachedManualDestination() const;

	// 현재 상태에서 새로운 행동(이동, 공격 등)이 가능한지 검증하는 플래그
	bool CanPerformAction() const;

	//------------------------------------------------------------------------------------------------------
public:
	// 외부(카메라, AI, 전투 컴포넌트)에서 접근하기 위한 Getter
	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	UActionEconomyComponent* GetActionEconomyComponent() const { return ActionEconomyComponent; }

	//	전투 컴포넌트를 가져오는 Getter
	UFUNCTION(BlueprintCallable, Category = "Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	//	장비 컴포넌트를 가져오는 Getter
	UFUNCTION(BlueprintCallable, Category = "Components")
	UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	//	버프 컴포넌트를 가져오는 Getter
	UFUNCTION(BlueprintCallable, Category = "Components")
	UBuffComponent* GetBuffComponent() const { return BuffComponent; }

	// 애니메이션 컴포넌트를 가져오는  Getter 추가
	UFUNCTION(BlueprintCallable, Category = "Components")
	UAnimationComponent* GetAnimationComponent() const { return AnimationComponent; }

	// AbilityComponent에 쉽게 접근할 수 있는 Getter
	UFUNCTION(BlueprintCallable, Category = "Components")
	UAbilityComponent* GetAbilityComponent() const { return AbilityComp; }

	// InteractComponent 게터
	UFUNCTION(BlueprintCallable, Category = "Components")
	UInteractComponent* GetInteractComponent() const { return InteractComponent; }

	// 부대장 등록 및 반환 게터
	void SetSquadManager(class ASquadManager* InSquadManager) { MySquadManager = InSquadManager; }
	ASquadManager* GetSquadManager() const { return MySquadManager; }

	// FlowField 이동 상태 확인용 Getter
	bool IsMovingWithFlowField() const { return bIsMovingWithFlowField; }

protected:	//	변수들
	// 선택 여부 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	bool bIsSelected = false;

	// 타입 설정을 테스트하기 위한 변수 (디폴트는 Friendly 또는 Enemy로 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EActionTargetType CharacterTargetType = EActionTargetType::Friendly;

	//	캐릭터 스텟,인스턴스에서 개별 수정 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	FBaseCharData CharacterData;

	// 수동 이동 / 강제 이동 상태 플래그 (공격보다 우선권 높음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsManualMoving = false;

	// Hold(제자리 사수) 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsHoldPosition = false;

	// 자동 전투 On/Off 플래그 (기본값: true)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAutoCombatEnabled = true;

	// 전투 진입 전 자동 전투 상태를 기억해둘 백업 변수
	bool bPreCombatAutoCombatEnabled = false;

	// 캐릭터가 수행할 현재 목표 액터 (적, 상자, NPC 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
	AActor* TargetActor = nullptr;

	// 현재 내린 명령의 종류 (Enemy: 공격, Interactable: 상호작용, Loot: 루팅 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
	EActionTargetType CurrentCommand = EActionTargetType::None;

	//	TurnParticipantInterface를 추가하면서 들어온 내용들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnSystem")
	bool bIsMyTurn = true; // 비전투 상태에서는 기본 true

	// 캐릭터의 현재 애니메이션 상태 (기본값: IdleRun)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ECharacterAnimState CurrentAnimState = ECharacterAnimState::IdleRun;

	// ==========================================
	// 장비 데이터 소켓 (에디터에서 꽂아넣는 드롭다운 슬롯)
	// ==========================================

	// 몬스터/캐릭터 기본 스탯 데이터 테이블 소켓 (FBaseCharData 연동)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Socket")
	FDataTableRowHandle CharacterStatRow;

	// 프레임별 이동거리 계산용 이전 위치 기록
	FVector PreviousFrameLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	EFactionGroup FactionGroup = EFactionGroup::PlayerParty; // 기본값 (에디터 BP에서 적/아군에 맞게 변경)

	// 현재 조준 중인 스킬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CRPG|Ability")
	FDataTableRowHandle PendingSkillRow;

	//	브로드 케스트 변수
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	// Flow Field 기반 이동 활성화 플래그
	bool bIsMovingWithFlowField = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Squad")
	ASquadManager* MySquadManager = nullptr;

protected:	//	컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TempMesh;

	// 선택 시 발밑에 켜질 데칼 컴포넌트 등
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDecalComponent> SelectionDecal;

	// 행동경제 자원 관리 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UActionEconomyComponent> ActionEconomyComponent;

	//	전투 함수가 들어있는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	//	장비 함수가 들어있는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	//	버프 함수가 들어있는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBuffComponent> BuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAnimationComponent> AnimationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityComponent> AbilityComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractComponent> InteractComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

};
