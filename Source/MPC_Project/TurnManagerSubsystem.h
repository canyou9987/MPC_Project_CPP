// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "TurnParticipantInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "TurnManagerSubsystem.generated.h"

// 턴 및 라운드 변경 알림 델리게이트 (UI 연동용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTurnChangedSignature, AActor*, CurrentTurnActor, int32, CurrentRound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChangedSignature, bool, bIsCombatActive);

UCLASS()
class MPC_PROJECT_API UTurnManagerSubsystem :  public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "TurnSystem|Events")
	FOnTurnChangedSignature OnTurnChanged;

	UPROPERTY(BlueprintAssignable, Category = "TurnSystem|Events")
	FOnCombatStateChangedSignature OnCombatStateChanged;

protected:
	// 특정 캐릭터 클래스가 아닌 인터페이스 액터 대기열!
	UPROPERTY()
	TArray<AActor*> TurnOrderQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnSystem")
	int32 CurrentTurnIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnSystem")
	int32 CurrentRound = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnSystem")
	bool bIsCombatActive = false;

public:
	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void StartCombatInRange(FVector CenterLocation, float CombatRadius);

	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void StartCombatWithActors(const TArray<AActor*>& Candidates);

	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void NextTurn();

	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void EndCombat();

	UFUNCTION(BlueprintPure, Category = "TurnSystem")
	AActor* GetCurrentTurnActor() const;

	UFUNCTION(BlueprintPure, Category = "TurnSystem")
	bool IsCombatActive() const { return bIsCombatActive; }

	UFUNCTION(BlueprintPure, Category = "TurnSystem")
	int32 GetCurrentRound() const { return CurrentRound; }

private:
	void RollInitiativeAndSort(const TArray<AActor*>& RawCandidates);
};
