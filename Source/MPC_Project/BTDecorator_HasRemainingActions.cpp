// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_HasRemainingActions.h"
#include "AIController.h"
#include "TurnParticipantInterface.h"

UBTDecorator_HasRemainingActions::UBTDecorator_HasRemainingActions()
{
	NodeName = TEXT("Has Remaining Actions");
}

bool UBTDecorator_HasRemainingActions::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return false;

	APawn* MyPawn = AICon->GetPawn();
	if (!MyPawn) return false;

	// BaseCharacter가 구현한 ITurnParticipantInterface의 HasRemainingActions() 호출
	if (ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(MyPawn))
	{
		return Participant->HasRemainingActions();
	}

	return false;
}
