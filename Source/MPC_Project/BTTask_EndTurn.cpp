// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EndTurn.h"
#include "AIController.h"
#include "TurnManagerSubsystem.h"

UBTTask_EndTurn::UBTTask_EndTurn()
{
	NodeName = TEXT("End Turn");
}

EBTNodeResult::Type UBTTask_EndTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return EBTNodeResult::Failed;

	if (UWorld* World = MyPawn->GetWorld())
	{
		if (UTurnManagerSubsystem* TurnSys = World->GetSubsystem<UTurnManagerSubsystem>())
		{
			if (TurnSys->IsCombatActive() && TurnSys->GetCurrentTurnActor() == MyPawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Action finished/Failed -> Forcing End Turn."), *MyPawn->GetName());
				TurnSys->NextTurn();
					return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
