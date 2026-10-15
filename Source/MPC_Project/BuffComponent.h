// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CRPGDataType.h"
#include "BuffComponent.generated.h"

USTRUCT(BlueprintType)
struct FActiveBuff
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FBuffDataRow BuffData;

	// 탐험 모드(실시간)용 만료 시각 (인게임 절대 시간 초)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ExpirationTime = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 버프 추가
	UFUNCTION(BlueprintCallable, Category = "Buff System")
	void ApplyBuff(FDataTableRowHandle BuffRow);

	// 버프 제거
	UFUNCTION(BlueprintCallable, Category = "Buff System")
	void RemoveBuff(FText BuffName);

	// 휴식(Rest) 등 강제 시간 경과 시 호출
	//void OnGameRoundTick(int32 RoundsPassed);

	// 턴제 전투 시 턴 시작할 때 호출
	//void OnCombatTurnStarted();

	UFUNCTION()
	void OnBuffTimeAdvanced(int32 RoundsPassed);

	// 최종 버프 스탯 합산 Getter
	int32 GetTotalAttackRollBonus() const;
	int32 GetTotalACBonus() const;
	int32 GetTotalMoveSpeedBonus() const;
		
protected:
	// 버프 지속시간 차감 및 만료 제거 공통 내부 로직
	//void DeductBuffDuration(int32 RoundsToDeduct);

	// 탐험 모드 실시간 만료 검사
	void CheckBuffExpiration();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff System")
	TArray<FActiveBuff> ActiveBuffs;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
};
