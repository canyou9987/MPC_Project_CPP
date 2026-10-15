// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AutoEquip.h"
#include "AIController.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"

UBTTask_AutoEquip::UBTTask_AutoEquip()
{
	NodeName = TEXT("Auto Equip Best Items");
}

EBTNodeResult::Type UBTTask_AutoEquip::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn()) return EBTNodeResult::Failed;

	APawn* MyPawn = AICon->GetPawn();
	UEquipmentComponent* EquipComp = MyPawn->FindComponentByClass<UEquipmentComponent>();
	UInventoryComponent* InvComp = MyPawn->FindComponentByClass<UInventoryComponent>();

	if (!EquipComp || !InvComp) return EBTNodeResult::Failed;

	const TArray<FInventorySlotInfo>& InvSlots = InvComp->GetInventorySlots();

	for (int32 i = 0; i < InvSlots.Num(); ++i)
	{
		if (InvSlots[i].IsEmpty()) continue;

		const FItemMasterDataRow& CandidateItem = InvSlots[i].ItemData;

		// 1. 장비 컴포넌트에게 성능 확인
		if (EquipComp->IsBetterItem(CandidateItem))
		{
			FItemMasterDataRow UnequippedOldItem;

			// 2. 실제 장착
			if (EquipComp->EquipItemData(CandidateItem, UnequippedOldItem))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s][BTTask_AutoEquip] Auto-Equipped: %s"),
					*MyPawn->GetName(), *CandidateItem.ItemName.ToString());

				// 3. 인벤토리 정리 (새 장비 차감 & 벗은 기존 장비 넣어주기)
				InvComp->RemoveItemAtIndex(i, 1);

				// EquipSlot뿐만 아니라 ItemID가 None이 아닐 때만(진짜 아이템이 벗겨졌을 때만) 인벤토리에 추가!
				if (UnequippedOldItem.EquipSlot != EEquipmentSlot::None && !UnequippedOldItem.ItemID.IsNone())
				{
					InvComp->AddItem(UnequippedOldItem, 1);
				}
			}
		}
	}

	return EBTNodeResult::Succeeded;
}
