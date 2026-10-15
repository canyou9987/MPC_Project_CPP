// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "InventoryHolderInterface.h"


// Sets default values
AFieldItemActor::AFieldItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 카메라 클릭 및 시선 트레이스 수용 콜리전
	MeshComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));

}

// Called when the game starts or when spawned
void AFieldItemActor::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에 직접 배치된 경우 초기화
	if (!ItemDataRow.IsNull())
	{
		InitializeItem(ItemDataRow, Quantity);
	}

}

void AFieldItemActor::InitializeItem(const FDataTableRowHandle& ItemRow, int32 InQuantity)
{
	if (ItemRow.IsNull()) return;

	const FItemMasterDataRow* FoundRow = ItemRow.GetRow<FItemMasterDataRow>(TEXT("FieldItem Init"));
	if (FoundRow)
	{
		InitializeItemFromData(*FoundRow, InQuantity);
	}
}

void AFieldItemActor::InitializeItemFromData(const FItemMasterDataRow& InItemData, int32 InQuantity)
{
	ItemData = InItemData;
	Quantity = InQuantity;

	// FItemMasterDataRow의 WorldMesh 지정
	if (ItemData.WorldMesh)
	{
		MeshComp->SetStaticMesh(ItemData.WorldMesh);
	}

	UE_LOG(LogTemp, Warning, TEXT("[FieldItem] Initialized: %s (Qty: %d)"), *ItemData.ItemName.ToString(), Quantity);
}

FInteractionInfo AFieldItemActor::GetInteractionInfo() const
{
	FInteractionInfo Info;
	Info.InteractionType = EActionTargetType::Loot;
	Info.bRequiresSkillCheck = false; // 루팅은 주사위 체크 없이 즉시 수거
	Info.InteractionRange = 150.0f;    // 1.5m 사거리
	Info.bIsAlreadyInteracted = bPickedUp;
	return Info;
}

bool AFieldItemActor::OnInteract(AActor* Interactor)
{
	if (!Interactor || bPickedUp) return false;

	// 1. 인터페이스 존재 여부만 검사
	IInventoryHolderInterface* InvHolder = Cast<IInventoryHolderInterface>(Interactor);
	if (!InvHolder) return false;

	// 2. 컴포넌트 추출 없이 인터페이스 함수 직접 호출!
	bool bAdded = InvHolder->TryAddItemToInventory(ItemData, Quantity);

	if (bAdded)
	{
		bPickedUp = true;

		UE_LOG(LogTemp, Warning, TEXT("[%s] Picked up Field Item via Interface: %s (Qty: %d)"),
			*Interactor->GetName(), *ItemData.ItemName.ToString(), Quantity);

		Destroy();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] Inventory FULL! Cannot pick up %s"),
		*Interactor->GetName(), *ItemData.ItemName.ToString());

	return false;
}

FText AFieldItemActor::GetInteractionPrompt() const
{
	FString PromptString = FString::Printf(TEXT("Loot %s"), *ItemData.ItemName.ToString());
	return FText::FromString(PromptString);
}