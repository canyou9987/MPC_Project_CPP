// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_HasBetterEquipment.h"
#include "AIController.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"

UBTDecorator_HasBetterEquipment::UBTDecorator_HasBetterEquipment()
{
	NodeName = TEXT("Has Better Equipment In Inventory?");
}

bool UBTDecorator_HasBetterEquipment::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return false;

	APawn* MyPawn = AICon->GetPawn();
	if (!MyPawn) return false;

	UEquipmentComponent* EquipComp = MyPawn->FindComponentByClass<UEquipmentComponent>();
	UInventoryComponent* InvComp = MyPawn->FindComponentByClass<UInventoryComponent>();

	if (!EquipComp || !InvComp) return false;

	// 인벤토리 순회하며 더 좋은 아이템이 단 1개라도 있는지 조건 검사만 수행
	const TArray<FInventorySlotInfo>& InvSlots = InvComp->GetInventorySlots();
	for (const FInventorySlotInfo& Slot : InvSlots)
	{
		if (!Slot.IsEmpty() && EquipComp->IsBetterItem(Slot.ItemData))
		{
			return true; // 조건 성립! -> Task 실행으로 진입 허용
		}
	}

	return false; // 더 좋은 장비 없음 -> Task 실행 차단
}
