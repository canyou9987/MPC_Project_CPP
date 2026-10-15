// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CRPGDataType.h"
#include "CombatStatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatStatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MPC_PROJECT_API ICombatStatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 이번 공격에 적용될 최종 명중 보너스 (근접/원거리/주문 통합)
	virtual int32 GetAttackBonus(EWeaponRangeType AttackType = EWeaponRangeType::Melee) const = 0;

	// 이번 공격에 적용될 최종 피해 추가 보너스 (근력/민첩/주문 보정치 등)
	virtual int32 GetDamageBonus() const = 0;

	// D&D 3.5e 내성 굴림 보너스 조회 (체질/인내, 민첩/반사, 지혜/의지)
	virtual int32 GetSavingThrowBonus(ESavingThrowType SaveType) const = 0;
};
