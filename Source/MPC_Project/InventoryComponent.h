// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CRPGDataType.h"
#include "InventoryComponent.generated.h"

// 에디터 디테일 패널에서 기본 배치 아이템을 설정하기 위한 구조체
USTRUCT(BlueprintType)
struct FDefaultItemSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FDataTableRowHandle ItemRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};

// 인벤토리 슬롯 변경 시 UI에 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPC_PROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// --- 핵심 기능 함수 ---

		// 아이템 추가 (성공 시 true, 공간 부족 시 false)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(const FItemMasterDataRow& ItemData, int32 Quantity = 1);

	// 특정 인덱스 슬롯 아이템 제거/소모
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemAtIndex(int32 SlotIndex, int32 Quantity = 1);

	// 두 슬롯 간 아이템 위치 교환 (드래그 & 드롭용)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SwapSlots(int32 FromIndex, int32 ToIndex);

	// 인벤토리 슬롯 배열 접근자
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FInventorySlotInfo>& GetInventorySlots() const { return InventorySlots; }

	// 슬롯 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetCapacity() const { return Capacity; }

	// 특정 인덱스의 슬롯 정보를 반환하는 Getter (유효하지 않은 인덱스 시 nullptr 반환)
	const FInventorySlotInfo* GetSlotInfoAtIndex(int32 SlotIndex) const;

public:
	// UI에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	// 총 인벤토리 슬롯 수 (기본 20칸)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Capacity = 20;

	// 실시간 슬롯 정보 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlotInfo> InventorySlots;

	// 레벨에 배치된 캐릭터/상자별로 에디터에서 초기 아이템을 설정할 수 있는 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Setup")
	TArray<FDefaultItemSetup> DefaultInventoryItems;

};
