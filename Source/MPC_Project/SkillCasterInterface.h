// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/DataTable.h"
#include "CRPGDataType.h"
#include "SkillCasterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USkillCasterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API ISkillCasterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 슬롯 인덱스(0, 1, 2...)로 보유 스킬 조준 대기 세팅
	virtual void SetPendingSkillBySlot(int32 SlotIndex) = 0;

	// 스킬 시전 대기 모드 (Targeting Mode) 제어
	virtual void SetPendingSkill(const FDataTableRowHandle& SkillRow) = 0;
	virtual void ClearPendingSkill() = 0;
	virtual bool HasPendingSkill() const = 0;

	// 최종 스킬 시전 실행
	virtual bool ExecutePendingSkill(AActor* TargetActor, const FVector& TargetLocation) = 0;

	// 슬롯 인덱스로 스킬 데이터 정보 얻기 (컴포넌트 은닉)
	virtual bool GetSkillDataBySlotByCharacter(int32 SlotIndex, FSkillDataRow& OutSkillData) const = 0;
};
