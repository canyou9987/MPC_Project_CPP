// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

// 슬롯 클릭 시 인덱스를 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSlotClickedSignature, int32, SlotIndex);

class UImage;
class UTextBlock;
class UButton;

UCLASS()
class MPC_PROJECT_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// 슬롯 초기화 (슬롯 인덱스, 단축키 텍스트 표기용)
	void InitSlot(int32 InSlotIndex, const FText& InKeyText);

	// 스킬 데이터 세팅 (아이콘, 사용 가능 여부 등)
	void SetSkillData(UTexture2D* SkillIcon, bool bIsUsable);

	// 슬롯 클릭 이벤트
	UFUNCTION(BlueprintCallable, Category = "UI|Skill")
	void OnSlotClicked();

	UPROPERTY(BlueprintAssignable, Category = "Events|UI")
	FOnSkillSlotClickedSignature OnSkillSlotClicked;

protected:
	// 버튼을 통해 클릭 이벤트를 감지합니다 (UMG에 Button이 있다면 자동 바인딩)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_SlotButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_SkillIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_KeyName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_CooldownOverlay; // 비활성화/쿨다운용 어두운 커버

	int32 SlotIndex = -1;
};
