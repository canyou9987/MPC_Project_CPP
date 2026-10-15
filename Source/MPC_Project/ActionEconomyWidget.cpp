// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionEconomyWidget.h"
#include "ActionEconomyInterface.h" 
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UActionEconomyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateActionEconomyUI();
}

void UActionEconomyWidget::SetTargetActionEconomy(UObject* InTargetObject)
{
	// 기존에 바인딩된 델리게이트가 있다면 먼저 해제
	if (TargetActionEconomy && TargetActionEconomy->GetOnActionEconomyChangedDelegate())
	{
		TargetActionEconomy->GetOnActionEconomyChangedDelegate()->RemoveAll(this);
	}

	if (InTargetObject && InTargetObject->Implements<UActionEconomyInterface>())
	{
		TargetActionEconomy = Cast<IActionEconomyInterface>(InTargetObject);
	}
	else if (AActor* TargetActor = Cast<AActor>(InTargetObject))
	{
		if (UActorComponent* Comp = TargetActor->FindComponentByInterface(UActionEconomyInterface::StaticClass()))
		{
			TargetActionEconomy = Cast<IActionEconomyInterface>(Comp);
		}
		else
		{
			TargetActionEconomy = nullptr;
		}
	}
	else
	{
		TargetActionEconomy = nullptr;
	}

	// 새 타겟의 델리게이트에 UpdateActionEconomyUI 함수 바인딩!
	if (TargetActionEconomy && TargetActionEconomy->GetOnActionEconomyChangedDelegate())
	{
		TargetActionEconomy->GetOnActionEconomyChangedDelegate()->AddDynamic(this, &UActionEconomyWidget::UpdateActionEconomyUI);
	}

	UpdateActionEconomyUI(); // 최초 1회 즉시 갱신
}

void UActionEconomyWidget::UpdateActionEconomyUI()
{
	if (!TargetActionEconomy)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 1. 표준 행동 아이콘
	if (Img_StandardAction)
	{
		float Opacity = TargetActionEconomy->HasStandardAction() ? 1.0f : 0.2f;
		Img_StandardAction->SetRenderOpacity(Opacity);
	}

	// 2. 이동 행동 아이콘
	if (Img_MoveAction)
	{
		float Opacity = TargetActionEconomy->HasMoveAction() ? 1.0f : 0.2f;
		Img_MoveAction->SetRenderOpacity(Opacity);
	}

	// 3. 신속 행동 아이콘
	if (Img_SwiftAction)
	{
		float Opacity = TargetActionEconomy->HasSwiftAction() ? 1.0f : 0.2f;
		Img_SwiftAction->SetRenderOpacity(Opacity);
	}

	// 4. 이동 거리
	if (Text_MoveDistance)
	{
		float RemainingDist = TargetActionEconomy->GetRemainingMoveDistance();
		int32 RemainingFt = FMath::RoundToInt(RemainingDist / 30.0f); // cm -> ft 환산
		Text_MoveDistance->SetText(FText::FromString(FString::Printf(TEXT("%d ft"), RemainingFt)));
	}
}


