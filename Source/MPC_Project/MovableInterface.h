// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MovableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMovableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API IMovableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 특정 위치로 이동 명령
	virtual void MoveToLocation(const FVector& TargetLocation) = 0;

	// 이동 정지 명령
	virtual void StopMovement() = 0;
};
