// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CRPGDataType.h"
#include "AbilityComponent.generated.h"

// 스킬 목록 변경 시 UI 갱신용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillsChangedSignature);

// 애니메이션 노티파이 발동 전까지 시전 정보를 임시 저장하는 구조체
USTRUCT(BlueprintType)
struct FPendingSpellCast
{
	GENERATED_BODY()

	const FSkillDataRow* SkillData = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	bool bIsValid = false;

	void Clear()
	{
		SkillData = nullptr;
		TargetActor = nullptr;
		TargetLocation = FVector::ZeroVector;
		bIsValid = false;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityComponent();

	// --- [스킬 조준 대기 관리] ---
	void SetPendingSkill(const FDataTableRowHandle& SkillRow);
	void SetPendingSkillBySlot(int32 SlotIndex);
	void ClearPendingSkill();
	bool HasPendingSkill() const { return !PendingSkillRow.IsNull(); }

	// 우클릭 시 호출되는 스킬 시전 실행 창구
	bool ExecutePendingSkill(AActor* TargetActor, const FVector& TargetLocation);

	// AnimNotify_OnSpellCast 노티파이에서 호출해 줄 스킬 효과 실행 함수
	UFUNCTION(BlueprintCallable, Category = "CRPG|Ability")
	void ExecutePendingSpellEffect();

	// 1. 시전 가능 여부 검사 (ActionEconomy 자원 및 사거리)
	UFUNCTION(BlueprintCallable, Category = "CRPG|Ability")
	bool CanCastSkill(const FDataTableRowHandle& SkillRow, AActor* TargetActor, FVector TargetLocation) const;

	// 2. 스킬 시전 실행 (ActionEconomy 자원 실제 소모)
	UFUNCTION(BlueprintCallable, Category = "CRPG|Ability")
	bool CastSkill(const FDataTableRowHandle& SkillRow, AActor* TargetActor, FVector TargetLocation);

	// 3. 신규 스킬/주문 학습 및 보유 여부 확인
	UFUNCTION(BlueprintCallable, Category = "CRPG|Ability")
	void LearnSkill(FDataTableRowHandle NewSkillRow);

	UFUNCTION(BlueprintPure, Category = "CRPG|Ability")
	bool HasLearnedSkill(const FDataTableRowHandle& SkillRow) const;

	// 슬롯 인덱스로 FSkillDataRow 데이터 가져오기
	bool GetSkillDataBySlot(int32 SlotIndex, FSkillDataRow& OutSkillData) const;
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
public:
	// 보유 중인 스킬/주문 목록 (D&D 3.5e 방식)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRPG|Ability")
	TArray<FDataTableRowHandle> LearnedSkills;

	// 현재 타깃팅 조준 중인 스킬
	UPROPERTY(VisibleAnywhere, Category = "CRPG|Ability")
	FDataTableRowHandle PendingSkillRow;

	// 애니메이션 노티파이 대기용 데이터
	UPROPERTY()
	FPendingSpellCast PendingCastData;

	// 조준 중 회전 속도 (도/초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRPG|Ability")
	float SpellStanceRotateSpeed = 12.0f;

	//	브로드케스트 변수
	UPROPERTY(BlueprintAssignable, Category = "Events|Skill")
	FOnSkillsChangedSignature OnSkillsChanged;

protected:
	// ActionEconomyComponent 검사 및 소모 헬퍼
	bool CheckActionEconomyResource(class UActionEconomyComponent* ActionComp, EActionCostType CostType) const;
	bool ConsumeActionEconomyResource(class UActionEconomyComponent* ActionComp, EActionCostType CostType);

	// 마우스 커서 방향으로 회전시키는 헬퍼 함수
	void RotateTowardsMouseCursor(float DeltaTime);

	// 스킬 효과 집행
	void ApplySkillEffectToTarget(const FSkillDataRow* SkillData, AActor* TargetActor);
	void ApplySkillEffectToArea(const FSkillDataRow* SkillData, FVector CenterLocation);

protected:
	//	파일 내 독립 시각 이펙트 스폰 헬퍼 함수
	void SpawnSkillImpactVFX(const FSkillDataRow* SkillData, FVector SpawnLocation);
};
