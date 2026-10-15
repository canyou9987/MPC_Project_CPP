// Fill out your copyright notice in the Description page of Project Settings.


#include "GameTimeSubsystem.h"

#include "TurnManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UGameTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TotalRoundsPassed = 0;
	bIsPaused = false;
	PauseStartTime = 0.0f;
	TotalPausedDuration = 0.0f;
}

void UGameTimeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameTimeSubsystem::SetGameTimePaused(bool bPause)
{
	if (bIsPaused == bPause) return;

	bIsPaused = bPause;

	if (bIsPaused)
	{
		PauseStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}
	else
	{
		if (GetWorld())
		{
			TotalPausedDuration += (GetWorld()->GetTimeSeconds() - PauseStartTime);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameTime] Game Time Paused: %s"), bIsPaused ? TEXT("True") : TEXT("False"));
}

float UGameTimeSubsystem::GetCurrentGameTime() const
{
	UWorld* World = GetWorld();
	if (!World) return 0.0f;

	UTurnManagerSubsystem* TurnSys = World->GetSubsystem<UTurnManagerSubsystem>();

	// 턴제 전투 중일 때: 전투 시작 시점 시간 + (진행된 라운드 * 6초)만 반환 (실시간 흐름 멈춤)
	if (TurnSys && TurnSys->IsCombatActive())
	{
		return CombatStartTime + ConvertRoundsToSeconds(CombatRoundsAdvanced);
	}

	// 탐험 모드일 때: 실시간 흐름 반환
	float RawTime = bIsPaused ? PauseStartTime : World->GetTimeSeconds();
	return FMath::Max(0.0f, RawTime - TotalPausedDuration);
}

void UGameTimeSubsystem::AdvanceTimeRounds(int32 RoundsToAdvance)
{
	if (RoundsToAdvance <= 0) return;

	TotalRoundsPassed += RoundsToAdvance;

	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (TurnSys && TurnSys->IsCombatActive())
	{
		// 턴제 전투 중일 때는 전투 전용 라운드 카운터를 올림
		CombatRoundsAdvanced += RoundsToAdvance;
	}
	else
	{
		// 탐험/휴식 모드일 때는 오프셋으로 시간을 밀어줌
		float SecondsToAdvance = ConvertRoundsToSeconds(RoundsToAdvance);
		TotalPausedDuration -= SecondsToAdvance;
	}

	OnRoundTick.Broadcast(RoundsToAdvance);

	UE_LOG(LogTemp, Warning, TEXT("[GameTime] Advanced Time by %d Rounds | Total Rounds: %d"),
		RoundsToAdvance, TotalRoundsPassed);
}
