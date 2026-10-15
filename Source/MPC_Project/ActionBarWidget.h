// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActionBarWidget.generated.h"

class USkillSlotWidget;
class UHorizontalBox;

UCLASS()
class MPC_PROJECT_API UActionBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	// 선택한 캐릭터의 AbilityComponent 또는 스킬 목록을 받아와 액션 바 전체 갱신
	UFUNCTION(BlueprintCallable, Category = "UI|ActionBar")
	void SetTargetCaster(AActor* TargetActor);

	void UpdateActionBarUI();

	
protected:
	// 바인딩 핸들러 함수 선언 추가
	UFUNCTION()
	void OnSlotClickedHandler(int32 SlotIndex);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HB_SlotContainer;

	// 슬롯 생성용 TSubclassOf (에디터 BP에서 WBP_SkillSlot 지정)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USkillSlotWidget> SkillSlotClass;

	UPROPERTY()
	TArray<TObjectPtr<USkillSlotWidget>> SlotWidgetList;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

};
