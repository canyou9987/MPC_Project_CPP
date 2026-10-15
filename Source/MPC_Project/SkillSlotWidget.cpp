// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_SlotButton)
	{
		Btn_SlotButton->OnClicked.RemoveDynamic(this, &USkillSlotWidget::OnSlotClicked);
		Btn_SlotButton->OnClicked.AddDynamic(this, &USkillSlotWidget::OnSlotClicked);
	}
}

void USkillSlotWidget::InitSlot(int32 InSlotIndex, const FText& InKeyText)
{
	SlotIndex = InSlotIndex;
	if (Text_KeyName)
	{
		Text_KeyName->SetText(InKeyText);
	}
}

void USkillSlotWidget::SetSkillData(UTexture2D* SkillIcon, bool bIsUsable)
{
	if (Img_SkillIcon)
	{
		if (SkillIcon)
		{
			Img_SkillIcon->SetBrushFromTexture(SkillIcon);
			Img_SkillIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (Img_CooldownOverlay)
	{
		// 사용 불가능하거나 스킬이 없으면 어둡게 가림
		Img_CooldownOverlay->SetVisibility(bIsUsable ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
}

void USkillSlotWidget::OnSlotClicked()
{
	// 슬롯 클릭 시 해당 슬롯 스킬 준비(SetPendingSkillBySlot) 명령 전달 예정

	if (SlotIndex != -1)
	{
		OnSkillSlotClicked.Broadcast(SlotIndex);
	}

}
