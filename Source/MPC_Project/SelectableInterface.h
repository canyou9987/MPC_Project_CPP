// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
//#include "InteractableInterface.h"
#include "ActionTargetType.h"
#include "SelectableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USelectableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API ISelectableInterface 
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetSelected(bool bSelected) = 0;						//	선택 함수
	virtual bool IsSelected() const = 0;								//	선택이 됬는지 체크하는 함수
	//virtual void MoveToLocation(const FVector& TargetLocation) = 0;		//	선택한 액터를 옴기는 함수

	// 대상의 상호작용 정체성(Enemy, Ground, Loot 등) 반환
	virtual EActionTargetType GetActionTargetType() const = 0;

	// CameraPawn에서 전달하는 통합 명령 인터페이스
	virtual void CommandTargetAction(AActor* TargetActor, EActionTargetType TargetType) = 0;
	// [추가/오버로딩] 위치 좌표 포함 명령 (지면 이동, 위치 지정 스펠 등)
	virtual void CommandTargetAction(EActionTargetType TargetType, const FVector& TargetLocation) = 0;

	// Hold 및 자동전투 토글용 인터페이스 함수
	virtual void ToggleHoldPosition() = 0;
	virtual void ToggleAutoCombat() = 0;

	virtual void StartFlowFieldMovement() = 0;
	virtual void StopFlowFieldMovement() = 0;

	// 나를 통제하는 최상위 선택 객체(SquadManager 등)의 인터페이스 반환
	virtual ISelectableInterface* GetSelectionOwner() { return this; }
};

