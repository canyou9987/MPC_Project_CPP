// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactionTypes.h"
#include "FactionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UFactionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 두 액터 간의 상대적 태도 판정
	UFUNCTION(BlueprintCallable, Category = "Faction")
	EFactionAttitude GetAttitude(AActor* SourceActor, AActor* TargetActor) const;

	// 진영 그룹 대 진영 그룹의 관계 판정
	UFUNCTION(BlueprintCallable, Category = "Faction")
	EFactionAttitude GetFactionAttitude(EFactionGroup SourceGroup, EFactionGroup TargetGroup) const;

	// 액터의 진영 그룹을 구하는 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Faction")
	EFactionGroup GetActorFaction(AActor* TargetActor) const;

};
