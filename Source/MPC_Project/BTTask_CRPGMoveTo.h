// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"		// UBTTaskNode 상속
#include "BehaviorTree/BehaviorTreeTypes.h" // FBlackboardKeySelector 사용
#include "BTTask_CRPGMoveTo.generated.h"

UCLASS()
class MPC_PROJECT_API UBTTask_CRPGMoveTo : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_CRPGMoveTo();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Task 시작 시 초기화용 메모리 구조체
	struct FBTMoveToTaskMemory
	{
		FVector LastLocation;
		float TotalDistanceMoved;
		float MaxAllowedDistance;
	};

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMoveToTaskMemory); }

public:
	
	// 에디터 디테일 창에서 'TargetActor' 키를 선택할 수 있게 뚫어주는 변수!
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Node")
	float AcceptableRadius = 100.0f; // 목적인근 도달 인정 범위

};
