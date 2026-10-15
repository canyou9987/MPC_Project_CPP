// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsInWeaponRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatComponent.h"

UBTDecorator_IsInWeaponRange::UBTDecorator_IsInWeaponRange()
{
	NodeName = TEXT("Is In Weapon Range");
}

bool UBTDecorator_IsInWeaponRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BB) return false;

	APawn* MyPawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!MyPawn || !TargetActor) return false;

	if (UCombatComponent* CombatComp = MyPawn->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->CombatTarget = TargetActor;
		return CombatComp->IsTargetInWeaponRange();
	}
	return false;
}