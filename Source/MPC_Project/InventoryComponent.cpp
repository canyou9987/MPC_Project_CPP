// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. Capacity 크기만큼 빈 슬롯 명시적 초기화
	InventorySlots.Empty();
	for (int32 i = 0; i < Capacity; ++i)
	{
		FInventorySlotInfo NewSlot;
		NewSlot.ItemID = NAME_None;
		NewSlot.Quantity = 0;
		InventorySlots.Add(NewSlot);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Inventory] Initialized Slots Count: %d"), InventorySlots.Num());

	// 2. 에디터 설정 초기 아이템 주입
	for (const FDefaultItemSetup& DefaultSetup : DefaultInventoryItems)
	{
		if (DefaultSetup.ItemRow.DataTable && !DefaultSetup.ItemRow.RowName.IsNone())
		{
			static const FString ContextString(TEXT("InventoryComponent_DefaultItemSetup"));	
			if (FItemMasterDataRow* ItemData = DefaultSetup.ItemRow.DataTable->FindRow<FItemMasterDataRow>(DefaultSetup.ItemRow.RowName, ContextString))
			{
				bool bSuccess = AddItem(*ItemData, DefaultSetup.Quantity);
				UE_LOG(LogTemp, Warning, TEXT("[Inventory] Default Item Add Result (%s): %s"),
					*DefaultSetup.ItemRow.RowName.ToString(), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
			}
		}
	}
}

bool UInventoryComponent::AddItem(const FItemMasterDataRow& ItemData, int32 Quantity)
{
	// 1. ItemID 체크 및 디버그 방어 (ItemID가 None이면 RowName이라도 넣어줌)
	FName EffectiveItemID = ItemData.ItemID;
	if (EffectiveItemID.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[Inventory] AddItem Failed: ItemData.ItemID is NAME_None!"));
		return false;
	}

	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Inventory] AddItem Failed: Invalid Quantity (%d)"), Quantity);
		return false;
	}

	int32 RemainingQuantity = Quantity;

	// 2. 중첩 가능한 아이템 처리
	if (ItemData.bIsStackable)
	{
		for (int32 i = 0; i < InventorySlots.Num(); ++i)
		{
			if (InventorySlots[i].ItemID == EffectiveItemID)
			{
				int32 CurrentQty = InventorySlots[i].Quantity;
				int32 MaxStack = ItemData.MaxStackCount > 0 ? ItemData.MaxStackCount : 99;
				int32 SpaceLeft = MaxStack - CurrentQty;

				if (SpaceLeft > 0)
				{
					int32 AddAmount = FMath::Min(RemainingQuantity, SpaceLeft);
					InventorySlots[i].Quantity += AddAmount;
					RemainingQuantity -= AddAmount;

					if (RemainingQuantity <= 0)
					{
						OnInventoryUpdated.Broadcast();
						return true;
					}
				}
			}
		}
	}

	// 3. 빈 슬롯 찾아 넣기
	while (RemainingQuantity > 0)
	{
		int32 EmptyIndex = INDEX_NONE;
		for (int32 i = 0; i < InventorySlots.Num(); ++i)
		{
			if (InventorySlots[i].IsEmpty())
			{
				EmptyIndex = i;
				break;
			}
		}

		if (EmptyIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("[Inventory] Inventory Full! Could not add remaining: %d"), RemainingQuantity);
			OnInventoryUpdated.Broadcast();
			return false;
		}

		int32 MaxStack = ItemData.bIsStackable ? (ItemData.MaxStackCount > 0 ? ItemData.MaxStackCount : 99) : 1;
		int32 AddAmount = FMath::Min(RemainingQuantity, MaxStack);

		InventorySlots[EmptyIndex].ItemID = EffectiveItemID;
		InventorySlots[EmptyIndex].Quantity = AddAmount;
		InventorySlots[EmptyIndex].ItemData = ItemData;

		RemainingQuantity -= AddAmount;

		UE_LOG(LogTemp, Warning, TEXT("[Inventory] Added Item to Slot [%d]: %s (Qty: %d)"),
			EmptyIndex, *ItemData.ItemName.ToString(), AddAmount);
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItemAtIndex(int32 SlotIndex, int32 Quantity)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
	{
		return false;
	}

	if (Quantity <= 0) return false;

	InventorySlots[SlotIndex].Quantity -= Quantity;

	if (InventorySlots[SlotIndex].Quantity <= 0)
	{
		InventorySlots[SlotIndex] = FInventorySlotInfo(); // 슬롯 초기화
		UE_LOG(LogTemp, Warning, TEXT("[Inventory] Slot [%d] Cleared."), SlotIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory] Slot [%d] Remaining Qty: %d"), SlotIndex, InventorySlots[SlotIndex].Quantity);
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::SwapSlots(int32 FromIndex, int32 ToIndex)
{
	if (!InventorySlots.IsValidIndex(FromIndex) || !InventorySlots.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex) return false;

	InventorySlots.Swap(FromIndex, ToIndex);

	UE_LOG(LogTemp, Warning, TEXT("[Inventory] Swapped Slot [%d] <-> Slot [%d]"), FromIndex, ToIndex);

	OnInventoryUpdated.Broadcast();
	return true;
}

const FInventorySlotInfo* UInventoryComponent::GetSlotInfoAtIndex(int32 SlotIndex) const
{
	if (InventorySlots.IsValidIndex(SlotIndex))
	{
		return &InventorySlots[SlotIndex];
	}

	return nullptr;
}

