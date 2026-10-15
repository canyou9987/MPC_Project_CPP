// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ExecuteCRPGAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatComponent.h"
#include "TurnManagerSubsystem.h"
#include "TurnParticipantInterface.h"
#include "DamageableInterface.h"

UBTTask_ExecuteCRPGAttack::UBTTask_ExecuteCRPGAttack()
{
	NodeName = TEXT("Execute CRPG Attack");
}

EBTNodeResult::Type UBTTask_ExecuteCRPGAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return EBTNodeResult::Failed;

	APawn* MyPawn = AICon->GetPawn();
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(FName("TargetActor")));

	// 1. 타깃 존재 유효성 및 사망 여부 검사
	bool bIsDead = false;
	if (!TargetActor)
	{
		bIsDead = true;
	}
	else if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(TargetActor))
	{
		if (Damageable->GetCurrentHP() <= 0)
		{
			bIsDead = true;
		}
	}

	// 2. 죽은 타깃이면 블랙보드에서 지우고 즉시 Failed로 탈출!
	if (bIsDead)
	{
		BB->ClearValue(FName("TargetActor"));
		return EBTNodeResult::Failed;
	}

	if (!MyPawn) return EBTNodeResult::Failed;

	UCombatComponent* CombatComp = MyPawn->FindComponentByClass<UCombatComponent>();
	if (!CombatComp) return EBTNodeResult::Failed;

	// 3. 사거리 검사
	CombatComp->CombatTarget = TargetActor;
	if (!CombatComp->IsTargetInWeaponRange())
	{
		return EBTNodeResult::Failed;
	}

	// 4. 턴 및 전투 상태 검사
	bool bInCombat = false;
	bool bIsMyTurn = true;

	if (UWorld* World = MyPawn->GetWorld())
	{
		if (UTurnManagerSubsystem* TurnSys = World->GetSubsystem<UTurnManagerSubsystem>())
		{
			bInCombat = TurnSys->IsCombatActive();
			if (bInCombat)
			{
				bIsMyTurn = (TurnSys->GetCurrentTurnActor() == MyPawn);
			}
		}
	}

	// 턴 모드인데 내 턴이 아니면 즉시 실패 후 탈출!
	if (bInCombat && !bIsMyTurn) return EBTNodeResult::Failed;

	// 5. 실제 공격 요청
	bool bAttackSuccess = CombatComp->RequestAttackCommand(TargetActor, bInCombat, bIsMyTurn);

	if (bAttackSuccess)
	{
		// 1. 턴 모드일 때: 1.2초 후 턴 넘김 + Task 성공 완료
		if (bInCombat && MyPawn->GetWorld())
		{
			FTimerHandle TurnTimerHandle;
			UBehaviorTreeComponent* OwnerCompPtr = &OwnerComp;

			MyPawn->GetWorld()->GetTimerManager().SetTimer(TurnTimerHandle, [this, OwnerCompPtr, MyPawn]()
				{
					if (IsValid(MyPawn) && MyPawn->GetWorld())
					{
						if (UTurnManagerSubsystem* TurnSys = MyPawn->GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
						{
							if (TurnSys->IsCombatActive() && TurnSys->GetCurrentTurnActor() == MyPawn)
							{
								TurnSys->NextTurn();
							}
						}
					}

					if (IsValid(OwnerCompPtr))
					{
						FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
					}
				}, 1.2f, false);

			return EBTNodeResult::InProgress;
		}

		// 2. 탐험(실시간) 모드일 때: 공격 쿨타임(예: 1.0초) 동안 대기 후 FinishLatentTask 호출!
		if (MyPawn->GetWorld())
		{
			FTimerHandle RealtimeTimerHandle;
			UBehaviorTreeComponent* OwnerCompPtr = &OwnerComp;

			// 공격 속도/공격 모션 타임에 맞춰 딜레이 부여 (예: 1.0초)
			MyPawn->GetWorld()->GetTimerManager().SetTimer(RealtimeTimerHandle, [this, OwnerCompPtr]()
				{
					if (IsValid(OwnerCompPtr))
					{
						FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
					}
				}, 1.0f, false); // 원하는 실시간 공격 연타 간격(초) 설정

			return EBTNodeResult::InProgress; // 딜레이 동안 InProgress로 대기
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
