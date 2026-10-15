// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CRPGDataType.h"
#include "InteractableInterface.h"
#include "FieldItemActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class MPC_PROJECT_API AFieldItemActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFieldItemActor();

	// 데이터 테이블 기반으로 아이템 데이터 및 메시 초기화
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItem(const FDataTableRowHandle& ItemRow, int32 InQuantity = 1);

	// 구조체 데이터를 직접 넘겨서 초기화 (인벤토리 드롭 시 사용)
	void InitializeItemFromData(const FItemMasterDataRow& InItemData, int32 InQuantity = 1);

	// ==========================================
		// IInteractableInterface 순수 가상 함수 구현
		// ==========================================
	virtual EActionTargetType GetActionTargetType() const override { return EActionTargetType::Loot; }
	virtual bool CanInteract(AActor* Interactor) const override { return !bPickedUp; }
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool OnInteract(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// 상호작용 범위 감지 콜리전 (선택 사항)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// 에디터 배치용 아이템 데이터 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FDataTableRowHandle ItemDataRow;

	// 아이템 수량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Quantity = 1;

	// 실시간 아이템 마스터 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemMasterDataRow ItemData;

	// 중복 파밍 방지 플래그
	bool bPickedUp = false;
};
