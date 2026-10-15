// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameTimeSubsystem.generated.h"

// 6초(1라운드) 경과 시 발생하는 델리게이트 (파기)
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundTickSignature, int32, TotalRoundsPassed);

// 휴식(Rest) 또는 이벤트로 대량의 라운드가 강제로 경과할 때 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundTickSignature, int32, TotalRoundsPassed);

UCLASS()
class MPC_PROJECT_API UGameTimeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 6초(1라운드) 마다 모든 시스템(버프, 피로도 등)으로 쏘는 방송
	//UPROPERTY(BlueprintAssignable, Category = "GameTime")
	//FOnRoundTickSignature OnRoundTick;

public:
	// 휴식(Rest) 등으로 강제 시간 경과 시 발생하는 방송
	UPROPERTY(BlueprintAssignable, Category = "GameTime")
	FOnRoundTickSignature OnRoundTick;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -----------------------------------------------------------------
	// 1. 시간 스케일 및 일시정지 제어
	// -----------------------------------------------------------------
	void SetGameTimePaused(bool bPause);
	bool IsGameTimePaused() const { return bIsPaused; }

	// -----------------------------------------------------------------
	// 2. 중앙 시간 및 타임스탬프 서비스
	// -----------------------------------------------------------------
	// 현재 인게임 절대 시간(초) 반환
	UFUNCTION(BlueprintPure, Category = "GameTime")
	float GetCurrentGameTime() const;

	// -----------------------------------------------------------------
	// 3. 룰북 단위 변환 헬퍼 함수 (초 단위 환산)
	// -----------------------------------------------------------------
	UFUNCTION(BlueprintPure, Category = "GameTime|Convert")
	static float ConvertRoundsToSeconds(int32 Rounds) { return Rounds * 6.0f; }

	UFUNCTION(BlueprintPure, Category = "GameTime|Convert")
	static float ConvertMinutesToSeconds(int32 Minutes) { return Minutes * 60.0f; }

	UFUNCTION(BlueprintPure, Category = "GameTime|Convert")
	static float ConvertHoursToSeconds(int32 Hours) { return Hours * 3600.0f; }

	// -----------------------------------------------------------------
	// 4. 휴식(Rest) 시스템
	// -----------------------------------------------------------------
	// 휴식 등으로 시간을 강제로 경과시킴 (예: 8시간 휴식 = 4800 라운드 = 28800초)
	UFUNCTION(BlueprintCallable, Category = "GameTime")
	void AdvanceTimeRounds(int32 RoundsToAdvance);

protected:
	UPROPERTY(VisibleAnywhere, Category = "GameTime")
	int32 TotalRoundsPassed = 0;

	UPROPERTY(VisibleAnywhere, Category = "GameTime")
	bool bIsPaused = false;

	// 일시정지 및 시간 측정을 위한 내부 오프셋
	float PauseStartTime = 0.0f;
	float TotalPausedDuration = 0.0f;

	// 전투 시작 시점의 인게임 시간
	float CombatStartTime = 0.0f;
	// 전투 중 AdvanceTimeRounds로 전진된 라운드 수
	int32 CombatRoundsAdvanced = 0;

};
