// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CRPGDataType.h"
#include "EquipmentComponent.generated.h"

// 장비 상태 변경 알림용 C++ 델리게이트 (퍼포먼스 최적화를 위해 C++ 전용 가벼운 델리게이트 선언)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponDataChanged, const FWeaponDataRow&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnArmorDataChanged, const FArmorDataRow&);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 소켓에 꽂힌 데이터 테이블 핸들로 캐릭터 장비 전체 초기화
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void InitializeEquipmentFromSockets();

	//	인벤토리 컴포넌트가 무엇이든, 순수 아이템 DataRow만 넘겨받아 장착
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItemData(const FItemMasterDataRow& NewItemData, FItemMasterDataRow& OutUnequippedItem);

	// 특정 슬롯 장비 해제
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UnequipSlot(EEquipmentSlot SlotToUnequip, FItemMasterDataRow& OutUnequippedItem);

	// 특정 슬롯에 새 무기 장착
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipPrimaryWeapon(FDataTableRowHandle NewWeaponRow);

	// 특정 슬롯에 새 갑옷 장착
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipArmor(FDataTableRowHandle NewArmorRow);

	// 현재 캐싱된 장비 데이터 Getter
	const FWeaponDataRow& GetCurrentWeaponData() const { return CurrentWeaponData; }
	const FArmorDataRow& GetCurrentArmorData() const { return CurrentArmorData; }

	// 무기 속성 즉시 판별 헬퍼 함수들
	bool IsRangedWeapon() const { return CurrentWeaponData.WeaponProperties.bIsRanged; }			//	나 원거리야?
	bool IsFinesseWeapon() const { return CurrentWeaponData.WeaponProperties.bIsFinesse; }			//	나 기교 무기야?
	bool IsTwoHandedWeapon() const { return CurrentWeaponData.WeaponProperties.bIsTwoHanded; }		//	나 양손 무기야?

	void SetUnarmedDefaultData();

	UFUNCTION(BlueprintPure, Category = "Equipment|Evaluation")
	bool IsBetterItem(const FItemMasterDataRow& CandidateItem) const;

public:
	// ==========================================
	// 외부 구독용 델리게이트 (Combat, Anim, UI 등이 구독)
	// ==========================================
	FOnWeaponDataChanged OnWeaponDataChanged;
	FOnArmorDataChanged OnArmorDataChanged;

public:
	// ==========================================
	// 에디터 지정용 장비 데이터 소켓 (DT 핸들)
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Sockets")
	FDataTableRowHandle PrimaryWeaponRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Sockets")
	FDataTableRowHandle ArmorRow;

protected:
	// ==========================================
	// 캐싱된 장비 데이터
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Data")
	FWeaponDataRow CurrentWeaponData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Data")
	FArmorDataRow CurrentArmorData;

	// 슬롯별 착용 중인 마스터 아이템 캐시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Data")
	TMap<EEquipmentSlot, FItemMasterDataRow> EquippedItems;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
		
};
