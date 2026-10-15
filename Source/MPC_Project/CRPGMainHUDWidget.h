// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CRPGMainHUDWidget.generated.h"

class UActionEconomyWidget;
class UTextBlock;
class UProgressBar;
class AActor;
class UActionBarWidget;
class UInventoryWindowWidget;

UCLASS()
class MPC_PROJECT_API UCRPGMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 선택한 캐릭터나 턴 변경 시 전체 HUD 데이터 바인딩
	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void SetSelectedUnit(AActor* NewUnit);

protected:
	virtual void NativeConstruct() override;

	// HP 변경 시 실행될 UFUNCTION 콜백 함수
	UFUNCTION()
	void OnHealthChangedCallback(int32 CurrentHP, int32 MaxHP);

public:
	// 상호작용 안내 텍스트 출력
	void SetInteractionPromptText(const FText& InText, const FVector2D& ScreenPosition);

	// 상호작용 안내 텍스트 숨기기
	void ClearInteractionPromptText();

	// 메인 HUD 내 인벤토리 위젯 포인터 게터
	UInventoryWindowWidget* GetInventoryWindowWidget() const { return WBP_InventoryWindow; }

protected:
	// UMG 에디터에서 WBP_ActionEconomy를 배치하면 C++ 변수에 자동 바인딩됨!
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UActionEconomyWidget> WBP_ActionEconomy;

	// WBP_MainHUD 안에 있는 WBP_ActionBar 자식 위젯을 C++로 자동 바인딩!
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UActionBarWidget> WBP_ActionBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CharacterName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PB_HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HealthText;

	UPROPERTY()
	TObjectPtr<AActor> CurrentSelectedUnit;

	// 이전 구독 대상 보관 (구독 해제용)
	class IDamageableInterface* CurrentDamageable = nullptr;

protected:
	// UMG에 배치된 TextBlock 바인딩 (이름: TXT_InteractionPrompt)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_InteractionPrompt;

	// UMG의 WBP_InventoryWindow 위젯과 자동 C++ 바인딩
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UInventoryWindowWidget> WBP_InventoryWindow;

};
