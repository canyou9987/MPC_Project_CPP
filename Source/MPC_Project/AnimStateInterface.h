// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CRPGDataType.h"
#include "AnimStateInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAnimStateInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API IAnimStateInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 애니메이션 인스턴스가 액터에게 물어볼 핵심 정보를 받아옴
	virtual ECharacterAnimState GetCurrentAnimState() const = 0;
	virtual float GetCurrentMovementSpeed() const = 0;

	// 외부(CombatComponent 등)에서 액터의 애니메이션 상태를 변경하기 위한 창구 추가
	virtual void SetCurrentAnimState(ECharacterAnimState NewState) = 0;

	//	인터페이스로 몽타주를 바로 재생하는 함수
	virtual float PlayAnimationMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.0f, FName StartSectionName = NAME_None) = 0;
};
