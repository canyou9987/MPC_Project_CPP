// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CRPGMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ActionEconomyComponent.h"
#include "CombatComponent.h"
#include "TurnManagerSubsystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_CRPGMoveTo::UBTTask_CRPGMoveTo()
{
	NodeName = TEXT("CRPG Move To Target");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_CRPGMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BB) return EBTNodeResult::Failed;

	APawn* MyPawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!MyPawn || !TargetActor) return EBTNodeResult::Failed;

	// 1. 이미 사거리 내에 도착했는지 확인
	UCombatComponent* CombatComp = MyPawn->FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		CombatComp->CombatTarget = TargetActor;
		if (CombatComp->IsTargetInWeaponRange())
		{
			// 이미 공격 가능 거리면 이동할 필요 없이 성공 반환!
			return EBTNodeResult::Succeeded;
		}
	}

	// 2. 남은 이동 거리(AP/Move Distance) 체크
	float RemainingDist = 9999.0f;
	UActionEconomyComponent* ActionEconomy = MyPawn->FindComponentByClass<UActionEconomyComponent>();

	UTurnManagerSubsystem* TurnSys = MyPawn->GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	if (bInCombat && ActionEconomy)
	{
		RemainingDist = ActionEconomy->GetRemainingMoveDistance();
		if (RemainingDist <= 10.0f) // 이동 가능 거리가 거의 없으면 실패 반환
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Move: Remaining Move Distance depleted!"), *MyPawn->GetName());
			return EBTNodeResult::Failed;
		}
	}

	// 3. Task 메모리 초기화
	FBTMoveToTaskMemory* MyMemory = (FBTMoveToTaskMemory*)NodeMemory;
	MyMemory->LastLocation = MyPawn->GetActorLocation();
	MyMemory->TotalDistanceMoved = 0.0f;
	MyMemory->MaxAllowedDistance = RemainingDist;

	// 4. 언리얼 내비게이션 추적 이동 시작
	EPathFollowingRequestResult::Type RequestResult = AIController->MoveToActor(TargetActor, AcceptableRadius);

	if (RequestResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_CRPGMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTMoveToTaskMemory* MyMemory = (FBTMoveToTaskMemory*)NodeMemory;
	FVector CurrentLoc = MyPawn->GetActorLocation();

	// 1. 이동 거리 계산 및 AP 소모
	float FrameDist = FVector::Distance(MyMemory->LastLocation, CurrentLoc);
	if (FrameDist > 0.0f)
	{
		MyMemory->TotalDistanceMoved += FrameDist;
		MyMemory->LastLocation = CurrentLoc;

		//if (UActionEconomyComponent* ActionEconomy = MyPawn->FindComponentByClass<UActionEconomyComponent>())
		//{
		//	ActionEconomy->ConsumeMoveDistance(FrameDist);
		//}
	}
	// 2. 이번 턴 이동 한계치(AP) 도달 시 이동 멈추고 이동 Task '성공' 처리 (위치 이동 수행 완료)
	if (MyMemory->TotalDistanceMoved >= MyMemory->MaxAllowedDistance)
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 3. 내비게이션 경로 도달 완료 시 성공
	EPathFollowingStatus::Type Status = AIController->GetMoveStatus();
	if (Status == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
