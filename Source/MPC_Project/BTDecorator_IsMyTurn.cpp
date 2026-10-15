// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsMyTurn.h"
#include "AIController.h"
#include "TurnManagerSubsystem.h"

UBTDecorator_IsMyTurn::UBTDecorator_IsMyTurn()
{
	NodeName = TEXT("Is My Turn");

	// 조건 값이 바뀌면 즉시 하위 실행을 중단(Abort)하도록 세팅

	bAllowAbortChildNodes = true;
	bAllowAbortLowerPri = true;
}

bool UBTDecorator_IsMyTurn::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return false;

	UWorld* World = MyPawn->GetWorld();
	if (!World) return false;

	UTurnManagerSubsystem* TurnSys = World->GetSubsystem<UTurnManagerSubsystem>();
	if (!TurnSys) return false;

	// 턴제 전투가 아니면 기본 통과 (탐험 모드 자유 행동)
	if (!TurnSys->IsCombatActive())
	{
		return true;
	}

	// 2. 턴 모드라면, 현재 턴을 가진 액터가 '나(MyPawn)'일 때만 True!
	if (ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(MyPawn))
	{
		return Participant->IsMyTurn();
	}

	// 턴제 전투 중이라면 현재 턴 액터가 나인지 검사
	return (TurnSys->GetCurrentTurnActor() == MyPawn);
}
