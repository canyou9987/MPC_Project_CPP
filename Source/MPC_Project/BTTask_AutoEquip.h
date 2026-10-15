// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AutoEquip.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UBTTask_AutoEquip : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_AutoEquip();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
