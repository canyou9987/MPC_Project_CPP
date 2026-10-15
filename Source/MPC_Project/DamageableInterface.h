// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

// HP 변경 알림 델리게이트 (현재HP, 최대HP 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, CurrentHP, int32, MaxHP);

class MPC_PROJECT_API IDamageableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 데미지를 전달받는 공통 함수
	virtual void ApplyDamageAmount(int32 Damage, AActor* DamageCauser) = 0;

	// 회복 수용 (만피 Clamp, 회복 텍스트, 언데드 힐/딜 반전 처리 등)
	virtual void ApplyHealingAmount(int32 HealAmount, AActor* Healer) = 0;

	// 현재 체력을 반환하는 공통 함수 (생존/사망 및 HP 체크용)
	virtual int32 GetCurrentHP() const = 0;
	// MaxHP를 가져오는 함수 추가!
	virtual int32 GetMaxHP() const = 0;

	// 피격 대상의 Armor Class(방어도)를 반환하는 공통 함수
	virtual int32 GetArmorClass() const = 0;

	// UI가 HP 방송을 구독할 수 있도록 델리게이트 Getter 제공
	virtual FOnHealthChanged* GetOnHealthChangedDelegate() = 0;

};
