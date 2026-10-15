// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsMyTurn.generated.h"


UCLASS()
class MPC_PROJECT_API UBTDecorator_IsMyTurn : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_IsMyTurn();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
