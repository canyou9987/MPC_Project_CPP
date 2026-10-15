// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/DataTable.h"
#include "BuffableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBuffableInterface : public UInterface
{
	GENERATED_BODY()
};

class MPC_PROJECT_API IBuffableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 외부(포션, 주문, 함정 등)에서 버프를 부여할 때 호출하는 통합 창구 함수
	virtual void ApplyBuffToSelf(FDataTableRowHandle BuffRow) = 0;
};
