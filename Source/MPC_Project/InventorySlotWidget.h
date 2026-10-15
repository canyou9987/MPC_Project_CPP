// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CRPGDataType.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class MPC_PROJECT_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 슬롯 UI 데이터 갱신
	void UpdateSlotUI(const FInventorySlotInfo& SlotInfo, int32 InSlotIndex);

	int32 GetSlotIndex() const { return SlotIndex; }

protected:
	// 마우스 클릭 이벤트 감지 (클릭 처리)
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 드래그 시작 감지 및 DropOperation 생성
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 슬롯에 드롭되었을 때 (슬롯 교환 SwapSlots Execution)
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	// 부모 InventoryWindowWidget을 안전하게 찾아주는 헬퍼
	class UInventoryWindowWidget* GetParentWindow() const;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_Quantity;

	// 슬롯 인덱스 기억용 변수
	int32 SlotIndex = INDEX_NONE;

	FInventorySlotInfo CachedSlotInfo;



};
