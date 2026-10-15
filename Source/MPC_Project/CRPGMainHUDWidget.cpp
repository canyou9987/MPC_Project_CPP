// Fill out your copyright notice in the Description page of Project Settings.


#include "CRPGMainHUDWidget.h"
#include "ActionEconomyWidget.h"
#include "DamageableInterface.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "CombatStatInterface.h"
#include "ActionBarWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void UCRPGMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCRPGMainHUDWidget::OnHealthChangedCallback(int32 CurrentHP, int32 MaxHP)
{
	if (PB_HealthBar && MaxHP > 0)
	{
		PB_HealthBar->SetPercent((float)CurrentHP / (float)MaxHP);
	}

	if (Text_HealthText)
	{
		Text_HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP)));
	}
}

void UCRPGMainHUDWidget::SetInteractionPromptText(const FText& InText, const FVector2D& ScreenPosition)
{
	if (TXT_InteractionPrompt)
	{
		TXT_InteractionPrompt->SetText(InText);
		TXT_InteractionPrompt->SetVisibility(ESlateVisibility::HitTestInvisible);

		// CanvasPanelSlot을 이용해 스크린 좌표로 위젯 위치 이동
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TXT_InteractionPrompt->Slot))
		{
			// 뷰포트 스케일(DPI Scaling) 보정
			float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
			FVector2D ScaledPosition = ScreenPosition / ViewportScale;

			CanvasSlot->SetPosition(ScaledPosition);
		}
	}
}

void UCRPGMainHUDWidget::ClearInteractionPromptText()
{
	if (TXT_InteractionPrompt)
	{
		TXT_InteractionPrompt->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCRPGMainHUDWidget::SetSelectedUnit(AActor* NewUnit)
{
	// 1. 기존에 구독 중이던 HP 델리게이트 해제
	if (CurrentDamageable && CurrentDamageable->GetOnHealthChangedDelegate())
	{
		CurrentDamageable->GetOnHealthChangedDelegate()->RemoveAll(this);
		CurrentDamageable = nullptr;
	}

	CurrentSelectedUnit = NewUnit;

	if (!IsValid(CurrentSelectedUnit))
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 캐릭터 이름 표기
	if (Text_CharacterName)
	{
		Text_CharacterName->SetText(FText::FromString(CurrentSelectedUnit->GetName()));
	}

	// 2. HP 인터페이스 가져오기 및 델리게이트 구독
	if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(CurrentSelectedUnit))
	{
		CurrentDamageable = Damageable;

		// 최초 1회 UI 수치 즉시 반영
		OnHealthChangedCallback(Damageable->GetCurrentHP(), Damageable->GetMaxHP());

		// HP 변경 델리게이트 바인딩!
		if (Damageable->GetOnHealthChangedDelegate())
		{
			Damageable->GetOnHealthChangedDelegate()->AddDynamic(this, &UCRPGMainHUDWidget::OnHealthChangedCallback);
		}
	}

	// 3. 행동력 데이터 전달
	if (WBP_ActionEconomy)
	{
		WBP_ActionEconomy->SetTargetActionEconomy(CurrentSelectedUnit);
	}

	if (WBP_ActionBar)
	{
		WBP_ActionBar->SetTargetCaster(CurrentSelectedUnit);
	}
}
