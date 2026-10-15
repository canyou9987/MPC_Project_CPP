// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h" // FWeaponDataRow 사용
#include "CRPGDataType.h" // FWeaponDataRow 사용
#include "CombatComponent.generated.h"

// 전방 선언
class UActionEconomyComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 공격 명령 수신 및 검증 (성공 시 Tick 활성화)
	bool RequestAttackCommand(AActor* InTargetActor, bool bInCombat, bool bIsMyTurn);

	// 공격 명령 취소 및 Tick 비활성화
	void StopAttackCommand();

	// 실제 공격 실행 (명중/피해 굴림 + AP 소모)
	void ExecuteAttack(AActor* Target, bool bInCombat);

	// 쿨다운 검사
	bool CanAttack() const;

	// 타깃 및 명령 정리
	void ClearCombatTarget();

	// 거리 연산 보조
	float GetDistanceToTarget() const;
	bool IsTargetInWeaponRange() const;

	// 무기 데이터 설정
	void SetCurrentWeaponData(const FWeaponDataRow& NewWeaponData) { CurrentWeaponData = NewWeaponData; }

	// EquipmentComponent의 델리게이트 수신용 함수
	void HandleWeaponDataChanged(const FWeaponDataRow& NewWeaponData);

	// AnimNotify에서 호출해 줄 타격 / 원거리 발사 수신 이벤트 함수
	void OnHitImpactNotify();
	void OnSpawnProjectileNotify();

protected:
	// 소유자 캐릭터 이동/정지 제어 헬퍼
	void RequestOwnerMoveTo(const FVector& TargetLocation);
	void RequestOwnerStop();


public:
	// 소유자 액터 및 행동경제 컴포넌트 참조
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<UActionEconomyComponent> ActionEconomyComp;

	// 현재 공격 타깃 (모든 AActor 지원)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	// 추적 이동 상태 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	uint8 bIsMovingToAttack : 1;

	// 컴포넌트가 참조하는 무기 데이터 (장비 컴포넌트/캐릭터가 설정해줌)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	FWeaponDataRow CurrentWeaponData;

	// 공격 쿨다운 관리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 6.0f;

	float LastAttackTime = -999.0f;

protected:
	// 애니메이션 몽타주 및 노티파이 전용 보관 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY()
	TObjectPtr<AActor> PendingTarget = nullptr;

	bool bPendingIsHit = false;
	int32 PendingDamage = 0;
};
