// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBarWidget.h"
#include "SkillSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "SkillCasterInterface.h"

void UActionBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 기본 1~6번 슬롯 동적 동시 생성 (필요에 따라 개수 조정)
	if (HB_SlotContainer && SkillSlotClass && SlotWidgetList.Num() == 0)
	{
		HB_SlotContainer->ClearChildren();

		for (int32 i = 0; i < 6; ++i)
		{
			USkillSlotWidget* NewSlot = CreateWidget<USkillSlotWidget>(this, SkillSlotClass);
			if (NewSlot)
			{
				FText KeyText = FText::FromString(FString::FromInt(i + 1));
				NewSlot->InitSlot(i, KeyText);

				// 슬롯 클릭 이벤트 바인딩
				NewSlot->OnSkillSlotClicked.AddDynamic(this, &UActionBarWidget::OnSlotClickedHandler);

				// 딱 한 번만 UI 패널과 리스트에 추가!
				HB_SlotContainer->AddChildToHorizontalBox(NewSlot);
				SlotWidgetList.Add(NewSlot);
			}
		}
	}
}

void UActionBarWidget::SetTargetCaster(AActor* TargetActor)
{
	CurrentTargetActor = TargetActor;

	if (!IsValid(CurrentTargetActor))
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UpdateActionBarUI();
}

void UActionBarWidget::UpdateActionBarUI()
{
	// 선택된 액터가 ISkillCasterInterface를 구현했는지 확인
	ISkillCasterInterface* SkillCaster = Cast<ISkillCasterInterface>(CurrentTargetActor);
	if (!SkillCaster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionBar] SkillCaster Interface를 안 가지고 있음!"));
		return;
	}

	for (int32 i = 0; i < SlotWidgetList.Num(); ++i)
	{
		if (!SlotWidgetList.IsValidIndex(i) || !SlotWidgetList[i]) continue;

		FSkillDataRow SkillData;
		if (SkillCaster->GetSkillDataBySlotByCharacter(i, SkillData))
		{
			UTexture2D* SkillIcon = SkillData.Icon.Get();
			UE_LOG(LogTemp, Log, TEXT("[ActionBar] Slot %d 스킬 찾음: %s (Icon: %s)"),
				i, *SkillData.SkillName.ToString(), SkillIcon ? *SkillIcon->GetName() : TEXT("Null Icon"));

			SlotWidgetList[i]->SetSkillData(SkillIcon, true);
		}
		else
		{
			SlotWidgetList[i]->SetSkillData(nullptr, false);
		}
	}
}

void UActionBarWidget::OnSlotClickedHandler(int32 SlotIndex)
{
	if (!IsValid(CurrentTargetActor))
	{
		return;
	}

	// 선택된 유닛이 ISkillCasterInterface를 구현했는지 확인
	ISkillCasterInterface* SkillCaster = Cast<ISkillCasterInterface>(CurrentTargetActor);
	if (SkillCaster)
	{
		// 해당 슬롯 인덱스의 스킬을 시전 대기(SetPendingSkill) 상태로 전환!
		SkillCaster->SetPendingSkillBySlot(SlotIndex);
		UE_LOG(LogTemp, Log, TEXT("[ActionBar] %d 번 슬롯 클릭됨! 스킬 시전 대기 연동"), SlotIndex);
	}
}
