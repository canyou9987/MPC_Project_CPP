// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindTargetEnemy.h"
#include "CRPG_AIController.h"

UBTTask_FindTargetEnemy::UBTTask_FindTargetEnemy()
{
	NodeName = TEXT("Find Target Enemy");
}

EBTNodeResult::Type UBTTask_FindTargetEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACRPG_AIController* AIController = Cast<ACRPG_AIController>(OwnerComp.GetAIOwner());
	if (!AIController) return EBTNodeResult::Failed;

	if (AIController->FindAndSetTargetEnemy())
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
