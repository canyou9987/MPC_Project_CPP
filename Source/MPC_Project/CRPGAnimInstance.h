// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CRPGDataType.h" // ECharacterAnimState 열거형이 포함되있음
#include "CRPGAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MPC_PROJECT_API UCRPGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// AnimBP 시작 시 초기화
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 애니메이션 상태 업데이트 (Tick과 유사)
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
	// 1. 이동 속도 (BlendSpace 및 Idle/Run 전환용)
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	float GroundSpeed = 0.0f;

	// 2. 이동 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	bool bIsMoving = false;

	// 3. C++ 캐릭터에서 가져올 현재 애니메이션 상태
	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	ECharacterAnimState CurrentAnimState = ECharacterAnimState::IdleRun;

	// 소유자 캐릭터 참조 포인터 캐싱
	//UPROPERTY()
	//TObjectPtr<class ABaseCharacter> OwnerCharacter;

	// 몽타주 OnHitImpact 노티파이 발생 시 엔진이 자동 호출할 함수
	UFUNCTION()
	void AnimNotify_OnHitImpact();

	// 원거리 발사용 노티파이
	UFUNCTION()
	void AnimNotify_OnSpawnProjectile();

	// 주문 시전(스킬 발동) 노티파이
	UFUNCTION()
	void AnimNotify_OnSpellCast();
};
