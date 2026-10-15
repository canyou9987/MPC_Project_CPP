// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWindowWidget.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "InventoryHolderInterface.h"
#include "Components/UniformGridPanel.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UInventoryWindowWidget::SetTargetInventoryComponent(UInventoryComponent* InInventoryComp)
{
	if (TargetInventoryComp)
	{
		TargetInventoryComp->OnInventoryUpdated.RemoveAll(this);
	}

	TargetInventoryComp = InInventoryComp;

	if (TargetInventoryComp)
	{
		TargetInventoryComp->OnInventoryUpdated.AddDynamic(this, &UInventoryWindowWidget::RefreshInventoryUI);

		// 인스턴스 메모리 주소와 슬롯 개수 함께 출력!
		UE_LOG(LogTemp, Warning, TEXT("[UI Target Check] Comp Address: %p | Slot Count: %d"),
			TargetInventoryComp.Get(),
			TargetInventoryComp ? TargetInventoryComp->GetInventorySlots().Num() : 0);

		RefreshInventoryUI();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UI Target Check] TargetInventoryComp is NULL!"));
	}
}

void UInventoryWindowWidget::SetTargetInventoryHolder(IInventoryHolderInterface* InHolder)
{
	TargetInventoryHolder = InHolder;

	if (TargetInventoryHolder)
	{
		// 인터페이스를 통해 인벤토리 컴포넌트를 가져와 기존 바인딩 진행
		SetTargetInventoryComponent(TargetInventoryHolder->GetInventoryComponent());
	}
	else
	{
		SetTargetInventoryComponent(nullptr);
	}
}

void UInventoryWindowWidget::RefreshInventoryUI()
{
	// 1. 셋 중 하나라도 NULL이면 "누가 NULL인지" 정확히 에러 로그를 남기고 종료
	if (!TargetInventoryComp || !Grid_InventorySlots || !SlotWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[RefreshInventoryUI 실패!] TargetComp: %s | GridPanel: %s | SlotWidgetClass: %s"),
			TargetInventoryComp ? TEXT("Valid") : TEXT("NULL"),
			Grid_InventorySlots ? TEXT("Valid") : TEXT("NULL"),
			SlotWidgetClass ? TEXT("Valid") : TEXT("NULL"));
		return;
	}

	const TArray<FInventorySlotInfo>& Slots = TargetInventoryComp->GetInventorySlots();

	//  2. 슬롯 위젯 배열 개수가 C++ 데이터 개수(20개)와 안 맞으면 동적 생성
	if (SlotWidgetList.Num() != Slots.Num())
	{
		Grid_InventorySlots->ClearChildren();
		SlotWidgetList.Empty();

		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			UInventorySlotWidget* NewSlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
			if (NewSlotWidget)
			{
				int32 Row = i / ColumnsCount;
				int32 Col = i % ColumnsCount;

				Grid_InventorySlots->AddChildToUniformGrid(NewSlotWidget, Row, Col);
				SlotWidgetList.Add(NewSlotWidget);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[UI Slot Rebuilt] Slot Widget Count: %d"), SlotWidgetList.Num());
	}

	//  3. 생성된 각 슬롯 UI에 포션/아이템 데이터 주입 및 갱신
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (SlotWidgetList.IsValidIndex(i) && SlotWidgetList[i])
		{
			SlotWidgetList[i]->UpdateSlotUI(Slots[i], i);
		}
	}
}

bool UInventoryWindowWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 1. 부모 클래스(UUserWidget)의 NativeOnDrop을 호출하여 Slate 기본 드롭 처리를 수행함
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 2. 손에 들고 온 택배 상자(InOperation)가 유효한지 널 체크 Guard
	// (비어있다면 이 드롭은 무효 처리하고 false 반환)
	if (!InOperation) return false;

	// 3. 택배 상자 안에 들어있는 출발지(Payload) 정보를 가져와 'UInventorySlotWidget'으로 형변환
	UInventorySlotWidget* FromSlotWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
	if (!FromSlotWidget) return false;

	// 4. 아이템을 집어 올렸던 원래 출발지 슬롯의 번호(Index)를 구함
	int32 FromIndex = FromSlotWidget->GetSlotIndex();

	// 5. 슬롯과 슬롯 사이의 인벤토리 창 '바탕(배경)'에 마우스를 놓았을 때
	// C++ 백엔드 인터페이스(TargetInventoryHolder)를 사용해 지면(월드)으로 아이템을 즉시 버림!
	if (TargetInventoryHolder)
	{
		return TargetInventoryHolder->RequestDropItemToWorld(FromIndex, 1);
	}

	return false;
}
