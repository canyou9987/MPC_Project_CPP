// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "GameTimeSubsystem.h"
#include "TurnManagerSubsystem.h"
#include "Engine/World.h"
#include "AbilityComponent.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// ...
}

// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerActor = GetOwner();

	// 전투 라운드 전진이나 휴식 등으로 '세계 시간이 밀려날 때' 시간 검사를 실행하기 위해 바인딩
	if (UWorld* World = GetWorld())
	{
		if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
		{
			TimeSys->OnRoundTick.AddDynamic(this, &UBuffComponent::OnBuffTimeAdvanced);
		}
	}
	
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	
	// 탐험 모드(비전투)일 때만 실시간 타임스탬프 만료 검사를 수행합니다.
	if (!TurnSys || !TurnSys->IsCombatActive())
	{
		CheckBuffExpiration();
	}
}

void UBuffComponent::ApplyBuff(FDataTableRowHandle BuffRow)
{
	if (BuffRow.IsNull()) return;
	FBuffDataRow* Data = BuffRow.GetRow<FBuffDataRow>(TEXT("Apply Buff Context"));
	if (!Data) return;

	UWorld* World = GetWorld();
	float CurrentGameTime = 0.0f;
	if (World)
	{
		if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
		{
			CurrentGameTime = TimeSys->GetCurrentGameTime();
		}
		else
		{
			CurrentGameTime = World->GetTimeSeconds();
		}
	}

	// 라운드(DurationRounds)는 '초'로 변환하는 입력 단위로만 사용함 (예: 3라운드 -> 18.0초)
	float DurationInSeconds = UGameTimeSubsystem::ConvertRoundsToSeconds(Data->DurationRounds);
	float TargetExpirationTime = CurrentGameTime + DurationInSeconds;

	// 기존 동일 버프 리프레시 (만료 시각만 새로 저장)
	for (FActiveBuff& Active : ActiveBuffs)
	{
		if (Active.BuffData.BuffName.EqualTo(Data->BuffName))
		{
			Active.ExpirationTime = TargetExpirationTime;
			UE_LOG(LogTemp, Warning, TEXT("[%s] Refreshed Buff: %s (Expires At: %.1fs)"),
				OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
				*Data->BuffName.ToString(), TargetExpirationTime);
			return;
		}
	}

	// 신규 버프 등록 (만료 시각 저장)
	FActiveBuff NewBuff;
	NewBuff.BuffData = *Data;
	NewBuff.ExpirationTime = TargetExpirationTime;
	ActiveBuffs.Add(NewBuff);

	UE_LOG(LogTemp, Warning, TEXT("[%s] Applied Buff: %s (Expires At: %.1fs)"),
		OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
		*Data->BuffName.ToString(), TargetExpirationTime);
}

void UBuffComponent::RemoveBuff(FText BuffName)
{
	ActiveBuffs.RemoveAll([&BuffName](const FActiveBuff& Buff) {
		return Buff.BuffData.BuffName.EqualTo(BuffName);
		});
}

void UBuffComponent::OnBuffTimeAdvanced(int32 RoundsPassed)
{
	// 라운드를 깎는 게 아니라 "현재 시간이 만료 시각을 넘었는가?"를 직접 체크!
	CheckBuffExpiration();
}

//void UBuffComponent::OnGameRoundTick(int32 RoundsPassed)
//{
//	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
//
//	// 전투 중일 때는 월드 틱에 의한 일괄 차감을 수행하지 않고 턴에 맞춰 차감
//	if (TurnSys && TurnSys->IsCombatActive()) return;
//
//	// 휴식(Rest) 시 AdvanceTimeRounds에 의해 불려온 경우 일괄 라운드 차감
//	DeductBuffDuration(RoundsPassed);
//}

//void UBuffComponent::OnCombatTurnStarted()
//{
//	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
//
//	// 턴제 전투 중일 때는 턴이 시작될 때마다 1라운드 차감
//	if (TurnSys && TurnSys->IsCombatActive())
//	{
//		DeductBuffDuration(1);
//	}
//}



int32 UBuffComponent::GetTotalAttackRollBonus() const
{
	int32 Total = 0;
	for (const FActiveBuff& Buff : ActiveBuffs)
	{
		Total += Buff.BuffData.AttackRollBonus;
	}
	return Total;
}

int32 UBuffComponent::GetTotalACBonus() const
{
	int32 Total = 0;
	for (const FActiveBuff& Buff : ActiveBuffs)
	{
		Total += Buff.BuffData.ACBonus;
	}
	return Total;
}

int32 UBuffComponent::GetTotalMoveSpeedBonus() const
{
	int32 Total = 0;
	for (const FActiveBuff& Buff : ActiveBuffs)
	{
		Total += Buff.BuffData.MoveSpeedModifier;
	}
	return Total;
}

//void UBuffComponent::DeductBuffDuration(int32 RoundsToDeduct)
//{
//	for (int32 i = ActiveBuffs.Num() - 1; i >= 0; --i)
//	{
//		ActiveBuffs[i].RemainingRounds -= RoundsToDeduct;
//
//		UE_LOG(LogTemp, Log, TEXT("[%s] Buff Round Tick: %s (Remaining: %d Rounds)"),
//			OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
//			*ActiveBuffs[i].BuffData.BuffName.ToString(),
//			ActiveBuffs[i].RemainingRounds);
//
//		if (ActiveBuffs[i].RemainingRounds <= 0)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("[%s] Buff Expired: %s"),
//				OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
//				*ActiveBuffs[i].BuffData.BuffName.ToString());
//
//			ActiveBuffs.RemoveAt(i);
//		}
//	}
//}

void UBuffComponent::CheckBuffExpiration()
{
	if (ActiveBuffs.Num() == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	float CurrentGameTime = 0.0f;
	if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
	{
		CurrentGameTime = TimeSys->GetCurrentGameTime();
	}
	else
	{
		CurrentGameTime = World->GetTimeSeconds();
	}

	//  오직 절대 시간(CurrentGameTime vs ExpirationTime) 비교로만 만료 검사!
	for (int32 i = ActiveBuffs.Num() - 1; i >= 0; --i)
	{
		if (CurrentGameTime >= ActiveBuffs[i].ExpirationTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("=================================================="));
			UE_LOG(LogTemp, Warning, TEXT("[%s] BUFF EXPIRED: %s (Current Time: %.2fs >= Expire At: %.2fs)"),
				OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
				*ActiveBuffs[i].BuffData.BuffName.ToString(),
				CurrentGameTime,
				ActiveBuffs[i].ExpirationTime);
			UE_LOG(LogTemp, Warning, TEXT("=================================================="));

			ActiveBuffs.RemoveAt(i);
		}
	}
}

