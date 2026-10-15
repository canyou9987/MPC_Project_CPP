// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTDecorator_IsInWeaponRange.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UBTDecorator_IsInWeaponRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_IsInWeaponRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

};
