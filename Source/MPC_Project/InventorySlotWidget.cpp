// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "InventoryHolderInterface.h" 
#include "InventoryWindowWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.h"
//#include "InventoryComponent.h"

void UInventorySlotWidget::UpdateSlotUI(const FInventorySlotInfo& SlotInfo, int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
	CachedSlotInfo = SlotInfo; // 전달받은 실시간 아이템 슬롯 정보를 캐싱!

	if (SlotInfo.IsEmpty())
	{
		if (IMG_ItemIcon) IMG_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (TXT_Quantity) TXT_Quantity->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 아이콘 출력
	if (IMG_ItemIcon)
	{
		if (SlotInfo.ItemData.ItemIcon)
		{
			IMG_ItemIcon->SetBrushFromTexture(SlotInfo.ItemData.ItemIcon);
			IMG_ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			IMG_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 수량 출력 (2개 이상이거나 중첩 가능한 경우에만 수량 표시)
	if (TXT_Quantity)
	{
		if (SlotInfo.Quantity > 1)
		{
			TXT_Quantity->SetText(FText::AsNumber(SlotInfo.Quantity));
			TXT_Quantity->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			TXT_Quantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

UInventoryWindowWidget* UInventorySlotWidget::GetParentWindow() const
{
	return Cast<UInventoryWindowWidget>(GetOuter()->GetOuter());
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// [디버그 1] 클릭된 마우스 버튼 이름 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("[SlotUI] Mouse Down Detected on Slot [%d] - Button: %s"),
		SlotIndex, *InMouseEvent.GetEffectingButton().ToString());

	// 1. 우클릭: 사용 요청
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (UInventoryWindowWidget* ParentWindow = Cast<UInventoryWindowWidget>(GetOuter()->GetOuter()))
		{
			if (IInventoryHolderInterface* Holder = ParentWindow->GetTargetInventoryHolder())
			{
				UE_LOG(LogTemp, Warning, TEXT("[SlotUI] Right Clicked Slot [%d] -> Requesting Item Use"), SlotIndex);
				Holder->RequestUseItemAtIndex(SlotIndex);
				return FReply::Handled();
			}
		}
	}

	// 2. 좌클릭: 드래그 감지
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SlotUI] Left Click Detected! Attempting DetectDragIfPressed..."));

		if (SlotIndex != INDEX_NONE && !CachedSlotInfo.IsEmpty())
		{
			FEventReply ReplyResult = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
			return ReplyResult.NativeReply;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SlotUI] Left Click Ignored: Slot is Empty or Invalid Index (%d)"), SlotIndex);
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// UItemDragDropOperation 으로 교체
	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	if (!DragOp) return;

	DragOp->Payload = this;

	// 드래그 비주얼 세팅
	UInventorySlotWidget* DragVisual = CreateWidget<UInventorySlotWidget>(GetWorld(), GetClass());
	if (DragVisual)
	{
		DragVisual->UpdateSlotUI(CachedSlotInfo, SlotIndex);
		DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
		DragOp->DefaultDragVisual = DragVisual;
		DragOp->Pivot = EDragPivot::MouseDown;
	}

	OutOperation = DragOp;
	UE_LOG(LogTemp, Warning, TEXT("[UI] Drag Detected on Slot Index: %d"), SlotIndex);
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation) return false;

	UInventorySlotWidget* FromSlotWidget = Cast<UInventorySlotWidget>(InOperation->Payload);	//	InOperation에 담긴 정보를 UInventorySlotWidget 형태로 지역변수에 저장한다.
	if (!FromSlotWidget || FromSlotWidget == this) return false;

	int32 FromIndex = FromSlotWidget->GetSlotIndex();											//	그정보를 열어서 처음 인덱스를 받아온다.
	int32 ToIndex = SlotIndex;																	//	목표 슬룻 정보

	// 인터페이스를 통해 부모 윈도우에게 스와프 요청!
	if (UInventoryWindowWidget* ParentWindow = Cast<UInventoryWindowWidget>(GetOuter()->GetOuter()))
	{
		if (IInventoryHolderInterface* Holder = ParentWindow->GetTargetInventoryHolder())
		{
			return Holder->RequestSwapSlots(FromIndex, ToIndex);
		}
	}

	return false;
}
