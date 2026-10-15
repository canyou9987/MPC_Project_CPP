// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
//#include "CombatComponent.h"
//#include "AnimationComponent.h"

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();

	// 게임 시작 시 에디터 소켓에 꽂힌 장비 데이터 로드 및 연동
	InitializeEquipmentFromSockets();
	
}


// Called every frame
void UEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UEquipmentComponent::InitializeEquipmentFromSockets()
{
	// 1. 주무기 로드
	if (!PrimaryWeaponRow.IsNull())
	{
		if (FWeaponDataRow* WeaponRow = PrimaryWeaponRow.GetRow<FWeaponDataRow>(TEXT("Primary Weapon Socket Context")))
		{
			CurrentWeaponData = *WeaponRow;
		}
	}
	else
	{
		//	지정무기가 없을때 비무장으로 세팅
		SetUnarmedDefaultData();
	}

	// 2. 갑옷 로드
	if (!ArmorRow.IsNull())
	{
		if (FArmorDataRow* ArmorRowData = ArmorRow.GetRow<FArmorDataRow>(TEXT("Armor Socket Context")))
		{
			CurrentArmorData = *ArmorRowData;
		}
	}

	// 초기화 시에도 방송
	OnWeaponDataChanged.Broadcast(CurrentWeaponData);
	OnArmorDataChanged.Broadcast(CurrentArmorData);
}

bool UEquipmentComponent::EquipItemData(const FItemMasterDataRow& NewItemData, FItemMasterDataRow& OutUnequippedItem)
{
	EEquipmentSlot TargetSlot = NewItemData.EquipSlot;
	if (TargetSlot == EEquipmentSlot::None) return false;

	// 1. 기존 착용 중인 슬롯에 장비가 있다면 해제 데이터 추출
	if (EquippedItems.Contains(TargetSlot))
	{
		OutUnequippedItem = EquippedItems[TargetSlot];
		EquippedItems.Remove(TargetSlot);
	}

	// 2. 새 장비 저장
	EquippedItems.Add(TargetSlot, NewItemData);

	// 3. DetailedDataRow에서 데이터 로드 및 델리게이트 전파
	if (NewItemData.ItemType == EItemType::Weapon)
	{
		if (FWeaponDataRow* NewWeaponRow = NewItemData.DetailedDataRow.GetRow<FWeaponDataRow>(TEXT("Equip Weapon Context")))
		{
			CurrentWeaponData = *NewWeaponRow;
			OnWeaponDataChanged.Broadcast(CurrentWeaponData); // 관심 있는 놈들 다 들어라!
		}
	}
	else if (NewItemData.ItemType == EItemType::Armor)
	{
		if (FArmorDataRow* NewArmorRow = NewItemData.DetailedDataRow.GetRow<FArmorDataRow>(TEXT("Equip Armor Context")))
		{
			CurrentArmorData = *NewArmorRow;
			OnArmorDataChanged.Broadcast(CurrentArmorData); // 방송!
		}
	}

	return true;
}

bool UEquipmentComponent::UnequipSlot(EEquipmentSlot SlotToUnequip, FItemMasterDataRow& OutUnequippedItem)
{
	if (!EquippedItems.Contains(SlotToUnequip)) return false;

	OutUnequippedItem = EquippedItems[SlotToUnequip];
	EquippedItems.Remove(SlotToUnequip);

	if (SlotToUnequip == EEquipmentSlot::MainHand)
	{
		SetUnarmedDefaultData();
		OnWeaponDataChanged.Broadcast(CurrentWeaponData);
	}
	else if (SlotToUnequip == EEquipmentSlot::Armor)
	{
		CurrentArmorData = FArmorDataRow();
		OnArmorDataChanged.Broadcast(CurrentArmorData);
	}

	return true;
}

void UEquipmentComponent::EquipPrimaryWeapon(FDataTableRowHandle NewWeaponRow)
{
	PrimaryWeaponRow = NewWeaponRow;

	if (!PrimaryWeaponRow.IsNull())
	{
		if (FWeaponDataRow* WeaponRow = PrimaryWeaponRow.GetRow<FWeaponDataRow>(TEXT("Equip Weapon Context")))
		{
			CurrentWeaponData = *WeaponRow;
		}
	}
	else
	{
		SetUnarmedDefaultData();
	}

	OnWeaponDataChanged.Broadcast(CurrentWeaponData);
}

void UEquipmentComponent::EquipArmor(FDataTableRowHandle NewArmorRow)
{
	if (NewArmorRow.IsNull()) return;

	ArmorRow = NewArmorRow;
	if (FArmorDataRow* ArmorRowData = ArmorRow.GetRow<FArmorDataRow>(TEXT("Equip Armor Context")))
	{
		CurrentArmorData = *ArmorRowData;
		OnArmorDataChanged.Broadcast(CurrentArmorData);
	}
}

// 맨손 디폴트 값 설정 헬퍼 함수
void UEquipmentComponent::SetUnarmedDefaultData()
{
	CurrentWeaponData.WeaponName = FText::FromString(TEXT("Unarmed Strike"));
	CurrentWeaponData.DamageDiceCount = 1;
	CurrentWeaponData.DamageDiceSides = 3;
	CurrentWeaponData.AttackRange = 100.0f;
	CurrentWeaponData.CriticalThreshold = 20;
	CurrentWeaponData.CriticalMultiplier = 2;

	//	무기 속성 초기화
	CurrentWeaponData.WeaponProperties.bIsRanged = false;
	CurrentWeaponData.WeaponProperties.bIsFinesse = false;
	CurrentWeaponData.WeaponProperties.bIsTwoHanded = false;
	CurrentWeaponData.WeaponProperties.bIsLight = true;
}

bool UEquipmentComponent::IsBetterItem(const FItemMasterDataRow& CandidateItem) const
{
	if (CandidateItem.EquipSlot == EEquipmentSlot::None) return false;

	// 1. 갑옷 성능 비교 (AC 보정치)
	if (CandidateItem.ItemType == EItemType::Armor)
	{
		if (const FArmorDataRow* CandidateArmor = CandidateItem.DetailedDataRow.GetRow<FArmorDataRow>(TEXT("Armor Eval Context")))
		{
			return CandidateArmor->ArmorClassBonus > CurrentArmorData.ArmorClassBonus;
		}
	}
	// 2. 무기 성능 비교 (주사위 평균 데미지 기댓값)
	else if (CandidateItem.ItemType == EItemType::Weapon)
	{
		if (const FWeaponDataRow* CandidateWeapon = CandidateItem.DetailedDataRow.GetRow<FWeaponDataRow>(TEXT("Weapon Eval Context")))
		{
			float CurrentAvgDmg = (CurrentWeaponData.DamageDiceCount * (CurrentWeaponData.DamageDiceSides + 1)) / 2.0f;
			float CandidateAvgDmg = (CandidateWeapon->DamageDiceCount * (CandidateWeapon->DamageDiceSides + 1)) / 2.0f;
			return CandidateAvgDmg > CurrentAvgDmg;
		}
	}

	return false;
}
