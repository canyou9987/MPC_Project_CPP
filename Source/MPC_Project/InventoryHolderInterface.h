// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CRPGDataType.h"
#include "InventoryHolderInterface.generated.h"

class UInventoryComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryHolderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API IInventoryHolderInterface
{
	GENERATED_BODY()

public:

	// 소유한 인벤토리 컴포넌트 반환
	virtual UInventoryComponent* GetInventoryComponent() const = 0;

	// 위젯이 컴포넌트를 직접 안 건드리고 인터페이스로 소모 요청!
	virtual bool RequestUseItemAtIndex(int32 SlotIndex, int32 Quantity = 1) = 0;

	// 컴포넌트 직접 참조 없이 아이템 추가를 다이렉트로 요청하는 인터페이스 함수!
	virtual bool TryAddItemToInventory(const FItemMasterDataRow& ItemData, int32 Quantity) = 0;

	// 인터페이스 기반 슬롯 스와프 및 월드 드롭 요청 함수
	virtual bool RequestSwapSlots(int32 FromIndex, int32 ToIndex) = 0;
	virtual bool RequestDropItemToWorld(int32 SlotIndex, int32 Quantity = 1) = 0;
};
