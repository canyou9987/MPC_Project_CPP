// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
protected:
	// 슬롯이 아닌 허공/창 바깥에 놓여 드래그가 취소되었을 때 자동으로 실행됨!
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
};
