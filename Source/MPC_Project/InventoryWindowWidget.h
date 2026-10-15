// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWindowWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class IInventoryHolderInterface;

UCLASS()
class MPC_PROJECT_API UInventoryWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 연동할 인벤토리 컴포넌트 지정 및 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void SetTargetInventoryComponent(UInventoryComponent* InInventoryComp);
	
	// 인터페이스 기반 타깃 설정 게터/세터
	void SetTargetInventoryHolder(IInventoryHolderInterface* InHolder);
	IInventoryHolderInterface* GetTargetInventoryHolder() const { return TargetInventoryHolder; }


	// 전체 인벤토리 UI 그리드 리프레시
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void RefreshInventoryUI();

	// 타깃 컴포넌트 Getter
	UInventoryComponent* GetTargetInventoryComponent() const { return TargetInventoryComp; }

	protected:
	// 인벤토리 창 바탕/허공 드롭 처리 (월드 아이템 버리기)
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory UI")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> Grid_InventorySlots;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> TargetInventoryComp;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgetList;

	// 열(Column) 개수 (예: 한 줄에 5칸씩)
	UPROPERTY(EditAnywhere, Category = "Inventory UI")
	int32 ColumnsCount = 5;

private:
	//	인터페이스
	IInventoryHolderInterface* TargetInventoryHolder = nullptr;

};
