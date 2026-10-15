// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TurnParticipantInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTurnParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API ITurnParticipantInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 1. 턴 활성화/비활성화 (내 턴 부여)
	virtual void SetTurnActive(bool bActive) = 0;
	virtual bool IsMyTurn() const = 0;

	// 2. 현재 체력 (생존 여부 체크)
	virtual int32 GetCurrentHP() const = 0;

	// 3. 우선권 보너스 (DEX 스탯 등)
	virtual int32 GetDexterityModifier() const = 0;

	// 4. 턴 종료 시 잔여 명령 초기화
	virtual void ClearCommand() = 0;

	// 5. 턴 시작/종료 이벤트 (추가된 부분)
	virtual void OnTurnStarted() = 0;
	virtual void OnTurnEnded() = 0;

	// 6. 턴 행동 자원 잔여 여부 (추가된 부분)
	virtual bool HasRemainingActions() const = 0;

	// 전투 종료 후 탐험 모드로 복구할 때 호출
	virtual void ResetToExplorationState() = 0;
};
