// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDragDropOperation.h"
#include "InventorySlotWidget.h"
#include "InventoryWindowWidget.h"
#include "InventoryHolderInterface.h"

void UItemDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);

	// 출발지 슬롯 정보 가져오기
	UInventorySlotWidget* FromSlot = Cast<UInventorySlotWidget>(Payload);
	if (!FromSlot) return;

	int32 FromIndex = FromSlot->GetSlotIndex();

	// 인터페이스를 통해 백엔드에 아이템 버리기 요청!
	if (UInventoryWindowWidget* ParentWindow = FromSlot->GetParentWindow())
	{
		if (IInventoryHolderInterface* Holder = ParentWindow->GetTargetInventoryHolder())
		{
			bool bDropped = Holder->RequestDropItemToWorld(FromIndex, 1);
			if (bDropped)
			{
				UE_LOG(LogTemp, Warning, TEXT("[UI Drag] Item Dropped to World! (Slot Index: %d)"), FromIndex);
			}
		}
	}
}