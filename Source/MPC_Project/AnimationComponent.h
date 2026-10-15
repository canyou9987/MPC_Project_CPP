// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CRPGDataType.h"
#include "AnimationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAnimationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 애니메이션 상태 관리
	void SetAnimState(ECharacterAnimState NewState);
	ECharacterAnimState GetAnimState() const { return CurrentAnimState; }

	// 무기 오버레이 포즈 관리
	void SetWeaponOverlayType(EWeaponOverlayType NewOverlay);
	EWeaponOverlayType GetWeaponOverlayType() const { return CurrentWeaponOverlay; }

	// 몽타주 재생 통합 제어
	float PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	// 현재 무기 데이터 기반 공격 몽타주 재생
	float PlayWeaponAttackMontage(UAnimMontage* DefaultMontageFallback = nullptr);

	// 무기 데이터 수신 (EquipmentComponent가 장비 교체 시 호출)
	void SetCurrentWeaponAnimData(UAnimMontage* AttackMontage, EWeaponOverlayType OverlayType);

	// 주문 시전 몽타주 재생 (PlayMontage로 위임)
	UFUNCTION(BlueprintCallable, Category = "CRPG|Animation")
	float PlayCastMontage(UAnimMontage* CastMontage, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CRPG|Animation")
	void StopCurrentMontage(float BlendOutTime = 0.2f);

	// EquipmentComponent 델리게이트 수신용 함수
	void HandleWeaponDataChanged(const FWeaponDataRow& NewWeaponData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|State")
	ECharacterAnimState CurrentAnimState = ECharacterAnimState::IdleRun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|State")
	EWeaponOverlayType CurrentWeaponOverlay = EWeaponOverlayType::Unarmed;

	// 캐싱된 무기 전용 공격 몽타주
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|Data")
	TObjectPtr<UAnimMontage> CurrentWeaponAttackMontage;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
		
};
