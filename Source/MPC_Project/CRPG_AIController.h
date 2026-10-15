// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CRPG_AIController.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API ACRPG_AIController : public AAIController
{
	GENERATED_BODY()

public:
	ACRPG_AIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

public:
	// 가장 가까운 적대 액터를 찾아 블랙보드 TargetActor 키에 세팅하는 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool FindAndSetTargetEnemy();

protected:
	// 에디터에서 지정할 비헤이비어 트리 및 블랙보드 자산
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBlackboardData> BlackboardAsset;

	// 블랙보드 키 이름 캐싱 (문자열 오타 방지)
	FName TargetActorKeyName = TEXT("TargetActor");
	FName bInCombatKeyName = TEXT("bInCombat");
	
};
