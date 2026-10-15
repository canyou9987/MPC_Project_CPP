// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/UserDefinedStruct.h"
#include "NiagaraSystem.h"					//	나이아가라 VFX
#include "Particles/ParticleSystem.h"		//	케스케이드 VFX
#include "CRPGDataType.generated.h"

//	장비 타입 정의
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None        UMETA(DisplayName = "None"),         // 장착 불가 또는 미지정
	MainHand    UMETA(DisplayName = "Main Hand"),
	OffHand     UMETA(DisplayName = "Off Hand"),
	TwoHand     UMETA(DisplayName = "Two Hand"),
	Armor       UMETA(DisplayName = "Armor"),
	Head        UMETA(DisplayName = "Head"),
	Ring        UMETA(DisplayName = "Ring"),
	Amulet      UMETA(DisplayName = "Amulet"),
	Boots       UMETA(DisplayName = "Boots")
};

//	주문 시전의 타입
UENUM(BlueprintType)
enum class ESpellTargetType : uint8
{
	SingleTarget    UMETA(DisplayName = "Single Target"),
	PointLocation   UMETA(DisplayName = "Point Location"),
	AreaOfEffect    UMETA(DisplayName = "Area Of Effect"),
	Self            UMETA(DisplayName = "Self Only")
};

//	공격 종류의 타입
UENUM(BlueprintType)
enum class EWeaponRangeType : uint8
{
	Melee    UMETA(DisplayName = "Melee"),
	Ranged   UMETA(DisplayName = "Ranged"),
	Spell	 UMETA(DisplayName = "Spell")
};

// 캐릭터의 현재 애니메이션 및 행동 상태
UENUM(BlueprintType)
enum class ECharacterAnimState : uint8
{
	IdleRun          UMETA(DisplayName = "Idle / Run"),          // 일반 대기 및 이동
	Attacking        UMETA(DisplayName = "Attacking"),          // 근접/원거리 공격 진행 중
	CastingSpell     UMETA(DisplayName = "Casting Spell"),      // 주문 시전 중
	HitReact         UMETA(DisplayName = "Hit Reaction"),       // 피격 모션 재생 중
	Stunned          UMETA(DisplayName = "Stunned / CC"),       // 기절/마비 등 행동 불능 상태
	Dead             UMETA(DisplayName = "Dead")                // 사망 상태
};

// 무기 보유/착용 유형 열거형
UENUM(BlueprintType)
enum class EWeaponOverlayType : uint8
{
	Unarmed        UMETA(DisplayName = "Unarmed / Bare Hands"),
	OneHanded      UMETA(DisplayName = "One-Handed Sword/Mace"),
	TwoHanded      UMETA(DisplayName = "Two-Handed Greatsword"),
	DualWield      UMETA(DisplayName = "Dual Wielding"),
	Bow            UMETA(DisplayName = "Bow / Crossbow"),
	Staff          UMETA(DisplayName = "Staff / Wand")
};

// 액터의 소속 진영
UENUM(BlueprintType)
enum class EFactionType : uint8
{
	PlayerParty    UMETA(DisplayName = "Player Party"),    // 플레이어 파티
	Hostile        UMETA(DisplayName = "Hostile Enemies"), // 적대 진영 (몬스터, 도적 등)
	Neutral        UMETA(DisplayName = "Neutral"),         // 중립 NPC
	MonsterFaction UMETA(DisplayName = "Wild Monsters")    // 제3의 야생 몬스터 (모두에게 적대)
};

// 두 진영 간의 관계
UENUM(BlueprintType)
enum class EFactionRelation : uint8
{
	Friendly, // 아군
	Hostile,  // 적대
	Neutral   // 중립
};

// 스킬 시전 시 필요한 행동 자원 타입
UENUM(BlueprintType)
enum class EActionCostType : uint8
{
	StandardAction UMETA(DisplayName = "Standard Action"),
	MoveAction     UMETA(DisplayName = "Move Action"),
	SwiftAction    UMETA(DisplayName = "Swift Action"),
	FreeAction     UMETA(DisplayName = "Free Action")
};

// 내성 굴림 스탯 타입 (D&D / Pathfinder 룰셋)
UENUM(BlueprintType)
enum class ESavingThrowType : uint8
{
	None        UMETA(DisplayName = "None"),
	Fortitude   UMETA(DisplayName = "Fortitude (체질/인내)"),
	Reflex      UMETA(DisplayName = "Reflex (민첩/반사)"),
	Will        UMETA(DisplayName = "Will (지혜/의지)")
};

// 스킬 판정 방식
UENUM(BlueprintType)
enum class ESkillExecutionType : uint8
{
	AttackRoll  UMETA(DisplayName = "Attack Roll vs AC"),   // 명중 굴림 (접촉 AC / 일반 AC)
	SavingThrow UMETA(DisplayName = "Saving Throw vs DC"),  // 내성 굴림
	BuffOnly    UMETA(DisplayName = "Buff / Utility"),      // 버프 / 자동 적용
	Heal        UMETA(DisplayName = "Heal")                 // 회복
};

// 아이템의 대분류 유형
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon          UMETA(DisplayName = "Weapon"),
	Armor           UMETA(DisplayName = "Armor"),
	Consumable      UMETA(DisplayName = "Consumable"),
	Scroll          UMETA(DisplayName = "Scroll"),
	Misc            UMETA(DisplayName = "Misc")
};

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	HealthPotion    UMETA(DisplayName = "Health Potion"),   // HP 회복 포션
	ManaPotion      UMETA(DisplayName = "Mana Potion"),     // MP/주문 슬롯 회복 포션
	Antidote        UMETA(DisplayName = "Antidote / Cure"), // 독/상태이상 해제제
	Elixir          UMETA(DisplayName = "Stat Elixir")      // 일시적 능력치 상승 영약
};

// 진영 관계 판정 헬퍼 유틸리티
namespace FactionUtils
{
	FORCEINLINE EFactionRelation GetRelation(EFactionType FactionA, EFactionType FactionB)
	{
		if (FactionA == FactionB) return EFactionRelation::Friendly;
		if (FactionA == EFactionType::Neutral || FactionB == EFactionType::Neutral) return EFactionRelation::Neutral;
		return EFactionRelation::Hostile;
	}
}

// ==========================================
// 1. 무기 세부 특성 및 데이터 (`FWeaponDataRow`)
// ==========================================

USTRUCT(BlueprintType)
struct FWeaponProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Property")
	bool bIsTwoHanded = false; // 양손 무기 (STR 보너스 1.5배 적용)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Property")
	bool bIsFinesse = false; // 기교 무기 (DEX 적용 가능)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Property")
	bool bIsLight = false; // 가벼운 무기

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Property")
	bool bIsRanged = false; // 원거리 무기
};

USTRUCT(BlueprintType)
struct FWeaponDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText WeaponName;

	// 피해 주사위 (예: 1d8, 2d6)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	int32 DamageDiceCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	int32 DamageDiceSides = 8;

	// 치명타 범위 및 배율 (예: 19 = 19~20 위협)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	int32 CriticalThreshold = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	int32 CriticalMultiplier = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FWeaponProperties WeaponProperties;

	// 원거리 무기일 경우 발사할 Projectile Class 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	TSubclassOf<class AProjectileActor> ProjectileClass;

	// 무기별 전용 공격 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> WeaponAttackMontage;

	// 무기 보유 형태 (Idle/Run 포즈 결정용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EWeaponOverlayType WeaponOverlayType = EWeaponOverlayType::Unarmed;
};

// ==========================================
// 2. 방어구 데이터 (`FArmorDataRow`)
// ==========================================

USTRUCT(BlueprintType)
struct FArmorDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText ArmorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EEquipmentSlot Slot = EEquipmentSlot::Armor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	int32 ArmorClassBonus = 4; // 기본 제공 AC

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor Limit")
	int32 MaxDexBonus = 2; // 최대 허용 민첩 보너스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor Limit")
	int32 ArmorCheckPenalty = -2; // 기술 페널티
};

// ==========================================
// 3. 버프/디버프 데이터 (`FBuffDataRow`)
// ==========================================

USTRUCT(BlueprintType)
struct FBuffDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText BuffName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	int32 DurationRounds = 3; // 지속 라운드 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers")
	int32 AttackRollBonus = 0; // 명중 보너스 (예: 축복 +1)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers")
	int32 ACBonus = 0;			// AC 보너스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers")
	int32 MoveSpeedModifier = 0;
};

// ==========================================
// 4. 통합 스킬/주문 데이터 (`FSkillDataRow`)
// ==========================================
USTRUCT(BlueprintType)
struct FSkillDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic")
	FText SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic")
	TObjectPtr<UTexture2D> Icon = nullptr;

	// --- ActionEconomyComponent 연동 코스트 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	EActionCostType ActionCost = EActionCostType::StandardAction;

	// --- 타깃팅 및 범위 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	ESpellTargetType TargetType = ESpellTargetType::SingleTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float CastRange = 1500.0f; // 시전 사거리 (cm)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float AoERadius = 0.0f; // 범위 폭발/영역 반경 (PointLocation, AreaOfEffect 전용)

	// --- 판정 및 주사위 룰 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ESkillExecutionType ExecutionType = ESkillExecutionType::AttackRoll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ESavingThrowType SaveType = ESavingThrowType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 DifficultyClass = 15; // 내성 굴림 DC

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 BaseDamageDiceCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 BaseDamageDiceSides = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FDataTableRowHandle AppliedBuffRow; // 버프/디버프 연동

	// [2단계] 스폰할 발사체 클래스 (AProjectileActor 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<AActor> ProjectileClass;

	// [Visual] 캐스케이드 기반 이펙트 (기존 / 레거시 리소스용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UParticleSystem> ImpactVFX;

	// [Visual] 나이아가라 기반 이펙트 (차세대 / 최신 리소스용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UNiagaraSystem> ImpactNiagaraVFX;

	// --- 애니메이션 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UAnimMontage> CastMontage = nullptr;

	// [신규] 주문 준비 자세 몽타주 (루프 재생용, 선택 사항)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UAnimMontage> CastLoopMontage;

	// [신규] 주문 발사/완료 몽타주 (OnSpellCast 노티파이가 포함된 몽타주)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UAnimMontage> CastReleaseMontage;
};

// ==========================================
// 5. 인벤토리 및 아이템 관리 구조체 (추가)
// ==========================================

// 인벤토리 마스터 아이템 데이터 (Data Table용)
USTRUCT(BlueprintType)
struct FItemMasterDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
	EItemType ItemType = EItemType::Misc;

	// 장비 아이템일 경우 착용될 슬롯 (CRPGDataType의 EEquipmentSlot 활용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EEquipmentSlot EquipSlot = EEquipmentSlot::Armor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TObjectPtr<UTexture2D> ItemIcon = nullptr;

	// 월드에 드랍/배치되었을 때 보여줄 3D 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
	bool bIsStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
	int32 MaxStackCount = 1;

	// 세부 무기/방어구 데이터 테이블 핸들 연동 (Weapon, Armor 전용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Data")
	FDataTableRowHandle DetailedDataRow;

	// 소비품(Consumable) / 두루마리(Scroll) 아이템 사용 시 발동할 스킬 데이터 소켓 (DT_Skill 연동)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Socket")
	FDataTableRowHandle LinkedSkillRow;

};

// 인벤토리 슬롯 실시간 데이터
USTRUCT(BlueprintType)
struct FInventorySlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FItemMasterDataRow ItemData;

	bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
};