// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.generated.h"

USTRUCT(BlueprintType)
struct FBaseCharData
{
	GENERATED_BODY()

	// ==========================================
	// 1. 6대 기본 스탯 (Ability Scores)
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 STR = 10; // 근력

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 DEX = 10; // 민첩

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 CON = 10; // 건강

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 INT = 10; // 지능

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 WIS = 10; // 지혜

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Ability")
	int32 CHA = 10; // 매력

	// ==========================================
	// 2. 전투 및 체력 데이터 (Combat Stats)
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Combat")
	int32 MaxHP = 20;				//	디폴트 HP값

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Combat")
	int32 CurrentHP = 20;			//	디폴트 현재값

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Combat")
	int32 BaseArmorClass = 10; // 기본 AC (방어력)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Combat")
	int32 BaseAttackBonus = 1; // BAB (기본 공격 보너스)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Combat")
	float AttackRange = 150.0f; // 공격 사거리 (근접: 150~200, 원거리: 800+)

	// ==========================================
	// 3. 스탯 보너스 계산 유틸리티 함수
	// ==========================================

	// 패스파인더 공식: (Stat - 10) / 2
	static int32 GetStatBonus(int32 StatValue)
	{
		return FMath::FloorToInt((StatValue - 10) / 2.0f);
	}

	// 각 스탯별 보너스 반환 편의 함수
	int32 GetSTRBonus() const { return GetStatBonus(STR); }
	int32 GetDEXBonus() const { return GetStatBonus(DEX); }
	int32 GetCONBonus() const { return GetStatBonus(CON); }
	int32 GetINTBonus() const { return GetStatBonus(INT); }
	int32 GetWISBonus() const { return GetStatBonus(WIS); }
	int32 GetCHABonus() const { return GetStatBonus(CHA); }

	// 최종 AC 계산 (기본 AC + DEX 보너스)
	int32 GetTotalAC(int32 ArmorACBonus = 0, int32 MaxDexBonus = 99) const
	{
		// 갑옷의 MaxDexBonus 제한을 적용한 실제 민첩 보너스 연산
		int32 EffectiveDexBonus = FMath::Min(GetDEXBonus(), MaxDexBonus);
		return BaseArmorClass + ArmorACBonus + EffectiveDexBonus;
	}

	// 근접 명중 보너스 (BAB + STR 보너스)
	int32 GetMeleeHitBonus() const { return BaseAttackBonus + GetSTRBonus(); }

	// 원거리 명중 보너스 (BAB + DEX 보너스)
	int32 GetRangedHitBonus() const { return BaseAttackBonus + GetDEXBonus(); }

	//

	// 내 순수 공격 사거리 반환 (UI 표시용)
	float GetBaseAttackRange() const { return AttackRange; }

	// 대상의 캡슐 반지름을 고려한 유효 공격 사거리 반환 (전투 판정용)
	float GetEffectiveAttackRange(float TargetCapsuleRadius = 0.0f) const
	{
		return AttackRange + TargetCapsuleRadius;
	}
};