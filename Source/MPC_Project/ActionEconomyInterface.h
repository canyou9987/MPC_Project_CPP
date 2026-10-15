// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionEconomyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UActionEconomyInterface : public UInterface
{
	GENERATED_BODY()
};

// 행동 자원 변경 알림 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionEconomyChanged);

class MPC_PROJECT_API IActionEconomyInterface
{
	GENERATED_BODY()

public:
	// 표준/이동/신속 행동 존재 여부
	virtual bool HasStandardAction() const = 0;
	virtual bool HasMoveAction() const = 0;
	virtual bool HasSwiftAction() const = 0;

	// 잔여 이동거리 (cm 또는 ft 단위)
	virtual float GetRemainingMoveDistance() const = 0;
	virtual float GetMaxMoveDistance() const = 0;

	// UI가 이 이벤트를 구독할 수 있도록 Getter 함수 제공
	virtual FOnActionEconomyChanged* GetOnActionEconomyChangedDelegate() = 0;
public:
};
