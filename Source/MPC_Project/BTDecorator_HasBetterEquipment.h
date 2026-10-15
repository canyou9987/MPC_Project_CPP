// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasBetterEquipment.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UBTDecorator_HasBetterEquipment : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HasBetterEquipment();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

};
