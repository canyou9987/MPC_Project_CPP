// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindTargetEnemy.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UBTTask_FindTargetEnemy : public UBTTaskNode
{
	GENERATED_BODY()

	UBTTask_FindTargetEnemy();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
