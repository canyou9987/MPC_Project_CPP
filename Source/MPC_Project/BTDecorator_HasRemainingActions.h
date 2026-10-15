// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasRemainingActions.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UBTDecorator_HasRemainingActions : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_HasRemainingActions();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
