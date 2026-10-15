// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerUIComponent.h"
#include "InteractableInterface.h"
#include "CRPGMainHUDWidget.h"
#include "InventoryHolderInterface.h"
#include "InventoryWindowWidget.h"
#include "GameFramework/PlayerController.h"

UPlayerUIComponent::UPlayerUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerUIComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateHoveredTargetUI();
}

void UPlayerUIComponent::InitializePlayerUI(APlayerController* InPC, UCRPGMainHUDWidget* InHUDWidget)
{
	PlayerController = InPC;
	MainHUDWidget = InHUDWidget;
}

void UPlayerUIComponent::ToggleInventoryUI()
{
	// 1. MainHUDWidget 유효성 체크
	if (!MainHUDWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[2. PlayerUIComponent] MainHUDWidget is NULL!"));
		return;
	}

	UInventoryWindowWidget* InvWindow = MainHUDWidget->GetInventoryWindowWidget();
	if (!InvWindow)
	{
		UE_LOG(LogTemp, Error, TEXT("[2. PlayerUIComponent] InventoryWindowWidget is NULL!"));
		return;
	}

	const bool bIsCurrentlyVisible = (InvWindow->GetVisibility() == ESlateVisibility::Visible);
	UE_LOG(LogTemp, Warning, TEXT("[2. PlayerUIComponent] Current Visibility State: %s"),
		bIsCurrentlyVisible ? TEXT("Visible") : TEXT("Collapsed/Hidden"));

	if (bIsCurrentlyVisible)
	{
		InvWindow->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("[2. PlayerUIComponent] -> Set to Collapsed"));
	}
	else
	{
		if (SelectedTargetActor)
		{
			if (IInventoryHolderInterface* InvHolder = Cast<IInventoryHolderInterface>(SelectedTargetActor))
			{
				InvWindow->SetTargetInventoryHolder(InvHolder);
				InvWindow->SetVisibility(ESlateVisibility::Visible);
				UE_LOG(LogTemp, Warning, TEXT("[2. PlayerUIComponent] -> Successfully set TargetHolder and Visible!"));
				return;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[2. PlayerUIComponent] SelectedTargetActor does NOT implement IInventoryHolderInterface!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[2. PlayerUIComponent] SelectedTargetActor is NULL!"));
		}
	}
}

void UPlayerUIComponent::SetSelectedTargetActor(AActor* InTargetActor)
{
	SelectedTargetActor = InTargetActor;

	if (!MainHUDWidget) return;

	UInventoryWindowWidget* InvWindow = MainHUDWidget->GetInventoryWindowWidget();
	if (!InvWindow) return;

	const bool bIsInvWindowVisible = (InvWindow->GetVisibility() == ESlateVisibility::Visible);

	// 1. 선택 해제되었거나(nullptr) 타깃이 없을 때
	if (!SelectedTargetActor)
	{
		if (bIsInvWindowVisible)
		{
			InvWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	// 2. 인벤토리 창이 켜져있는 상태에서 다른 캐릭터를 클릭한 경우
	if (bIsInvWindowVisible)
	{
		// 인터페이스를 가져와서 SetTargetInventoryHolder로 전달!
		if (IInventoryHolderInterface* InvHolder = Cast<IInventoryHolderInterface>(SelectedTargetActor))
		{
			InvWindow->SetTargetInventoryHolder(InvHolder);
			return;
		}

		// 만약 인벤토리가 없는 오브젝트/몬스터를 클릭했다면 창 닫기
		InvWindow->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerUIComponent::UpdateHoveredTargetUI()
{
	if (!PlayerController || !MainHUDWidget) return;

	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 1. 상호작용 가능 오브젝트 감지
			if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitActor))
			{
				FText PromptText = Interactable->GetInteractionPrompt();

				// 오브젝트 Bounding Box 기반 머리 위 높이 계산
				FVector BoxExtent;
				FVector Origin;
				HitActor->GetActorBounds(true, Origin, BoxExtent);
				FVector WorldPromptLocation = Origin + FVector(0.0f, 0.0f, BoxExtent.Z + 20.0f);

				// 3D 월드 좌표 -> 2D 스크린 좌표 변환
				FVector2D ScreenPosition;
				if (PlayerController->ProjectWorldLocationToScreen(WorldPromptLocation, ScreenPosition))
				{
					MainHUDWidget->SetInteractionPromptText(PromptText, ScreenPosition);
					return;
				}
			}
		}
	}

	// 감지된 오브젝트가 없으면 UI 정리
	MainHUDWidget->ClearInteractionPromptText();
}
