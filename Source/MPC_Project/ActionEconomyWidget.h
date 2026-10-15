// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActionEconomyWidget.generated.h"

class UImage;
class UTextBlock;
class IActionEconomyInterface;

UCLASS()
class MPC_PROJECT_API UActionEconomyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 구체 컴포넌트 대신 인터페이스를 구현한 대상(AActor 또는 UObject)을 받음!
	UFUNCTION(BlueprintCallable, Category = "UI|ActionEconomy")
	void SetTargetActionEconomy(UObject* InTargetObject);

	UFUNCTION(BlueprintCallable, Category = "UI|ActionEconomy")
	void UpdateActionEconomyUI();

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_StandardAction;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_MoveAction;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_SwiftAction;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_MoveDistance;

	// 인터페이스 포인터 보관
	IActionEconomyInterface* TargetActionEconomy = nullptr;
};
