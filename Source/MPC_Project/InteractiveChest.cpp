// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveChest.h"

// Sets default values
AInteractiveChest::AInteractiveChest()
{
	PrimaryActorTick.bCanEverTick = false;

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	RootComponent = ChestMesh;

	// 기본 상호작용 데이터 초기화 (잠긴 상자: DC 12)
	InteractionInfo.InteractionType = EActionTargetType::Interactable;
	InteractionInfo.bRequiresSkillCheck = true;
	InteractionInfo.DifficultyClass = 12; // 자물쇠 따기 난이도 12
	InteractionInfo.InteractionRange = 150.0f;
	InteractionInfo.bIsAlreadyInteracted = false;
}

// Called when the game starts or when spawned
void AInteractiveChest::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AInteractiveChest::CanInteract(AActor* Interactor) const
{
	// 열려있든 닫혀있든 플레이어가 클릭해서 다가오는 것 자체는 항상 허용!
	return true;
}

bool AInteractiveChest::OnInteract(AActor* Interactor)
{
	// 이미 열린 상태 (Loot 상태)인 경우 -> 주사위 굴림 없이 루팅 UI/아이템 획득 로직 실행
	if (bIsOpened)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Chest] %s accessed an already opened chest (Loot). Opening Loot UI."),
			Interactor ? *Interactor->GetName() : TEXT("None"));

		// TODO: 향후 Phase 2에서 루팅 UI 팝업 창 호출 (OpenLootUI)
		return true;
	}

	// 닫힌 상태인 경우 -> 상자 해제(열기) 집행
	bIsOpened = true;
	InteractionInfo.bIsAlreadyInteracted = true;
	InteractionInfo.bRequiresSkillCheck = false; // 한 번 따서 열었으면 스킬 체크 해제
	InteractionInfo.InteractionType = EActionTargetType::Loot; // 열린 후에는 Loot 타깃으로 전환

	UE_LOG(LogTemp, Warning, TEXT("======================================"));
	UE_LOG(LogTemp, Warning, TEXT("[Chest] %s successfully unlocked and opened the chest!"),
		Interactor ? *Interactor->GetName() : TEXT("None"));
	UE_LOG(LogTemp, Warning, TEXT("======================================"));

	return true;
}

FText AInteractiveChest::GetInteractionPrompt() const
{
	if (bIsOpened)
	{
		return FText::FromString(TEXT("Search Chest (Loot)"));
	}

	if (InteractionInfo.bRequiresSkillCheck)
	{
		return FText::Format(
			FText::FromString(TEXT("Open Chest (Lockpicking DC {0})")),
			FText::AsNumber(InteractionInfo.DifficultyClass)
		);
	}

	return FText::FromString(TEXT("Open Chest"));
}

