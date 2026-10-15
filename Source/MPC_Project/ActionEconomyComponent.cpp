// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionEconomyComponent.h"
#include "GameTimeSubsystem.h"
#include "TurnManagerSubsystem.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UActionEconomyComponent::UActionEconomyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UActionEconomyComponent::BeginPlay()
{
	Super::BeginPlay();

	// 휴식(Rest)으로 강제로 시간이 흐를 때 수신하기 위해 구독
	if (UWorld* World = GetWorld())
	{
		if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
		{
			TimeSys->OnRoundTick.AddDynamic(this, &UActionEconomyComponent::OnGameRoundTick);
		}
	}
	
}
void UActionEconomyComponent::OnGameRoundTick(int32 TotalRoundsPassed)
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();

	// 턴제 전투 중이 아닐 때, 휴식(Rest) 등으로 강제 라운드가 경과한 경우 자원 회복
	if (!TurnSys || !TurnSys->IsCombatActive())
	{
		ResetActionEconomy();
	}
}


void UActionEconomyComponent::ResetActionEconomy()
{
	bHasStandardAction = true;
	bHasMoveAction = true;
	bHasSwiftAction = true;
	RemainingMoveDistance = MaxMoveDistance;

	OnActionEconomyChanged.Broadcast();
	// 타이머 핸들 클리어
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Action Economy Replenished."), *GetOwner()->GetName());

}

bool UActionEconomyComponent::ConsumeStandardAction()
{
	if (!bHasStandardAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to consume Standard Action (No resource remaining)"),
			*GetOwner()->GetName());
		return false;
	}

	bHasStandardAction = false;
	OnActionEconomyChanged.Broadcast();
	StartExplorationCooldownTimer(); // 소모 시점 기준 회복 타이머 가동
	return true;
}

bool UActionEconomyComponent::ConsumeMoveAction()
{
	if (!bHasMoveAction) return false;

	bHasMoveAction = false;
	OnActionEconomyChanged.Broadcast();
	StartExplorationCooldownTimer();
	return true;
}

bool UActionEconomyComponent::ConsumeSwiftAction()
{
	if (!bHasSwiftAction) return false;

	bHasSwiftAction = false;
	OnActionEconomyChanged.Broadcast();
	StartExplorationCooldownTimer();
	return true;
}

bool UActionEconomyComponent::ConsumeMoveDistance(float Distance)
{
	if (RemainingMoveDistance < Distance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Insufficient move distance (Required: %.1f, Remaining: %.1f)"),
			*GetOwner()->GetName(), Distance, RemainingMoveDistance);
		return false;
	}

	RemainingMoveDistance -= Distance;

	UE_LOG(LogTemp, Warning, TEXT("[%s] ConsumeMoveDistance: -%.1f cm | Remaining: %.1f / %.1f cm"),
		*GetOwner()->GetName(), Distance, RemainingMoveDistance, MaxMoveDistance);

	OnActionEconomyChanged.Broadcast();
	StartExplorationCooldownTimer();
	return true;
}


void UActionEconomyComponent::StartExplorationCooldownTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UTurnManagerSubsystem* TurnSys = World->GetSubsystem<UTurnManagerSubsystem>();

	// 턴제 전투 중일 때는 턴 관리자(TurnManager)가 회복시키므로 타이머를 돌리지 않음
	if (TurnSys && TurnSys->IsCombatActive()) return;

	// GameTimeSubsystem에서 1라운드(6.0초) 기준 시간을 구해옴
	float CooldownDuration = 6.0f;
	if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
	{
		CooldownDuration = UGameTimeSubsystem::ConvertRoundsToSeconds(1); // 6.0초
	}

	// 회복 타이머가 돌아가고 있지 않다면, 소모된 시점 기준 정확히 6초 뒤 ResetActionEconomy 호출
	if (!World->GetTimerManager().IsTimerActive(CooldownTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UActionEconomyComponent::ResetActionEconomy,
			CooldownDuration,
			false // 단발성
		);

		UE_LOG(LogTemp, Log, TEXT("[%s] Exploration Action Cooldown Timer Started (%.1fs)"),
			*GetOwner()->GetName(), CooldownDuration);
	}
}

// Called every frame
void UActionEconomyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 행동력이 소모되었는지 체크하는 헬퍼 함수
//bool UActionEconomyComponent::IsActionEconomyConsumed() const
//{
//	// Standard, Move, Swift Action 중 하나라도 썼거나, 이동 거리를 조금이라도 썼다면 true
//	const bool bActionsUsed = (!bHasStandardAction || !bHasMoveAction || !bHasSwiftAction);
//	const bool bMoveDistanceUsed = (RemainingMoveDistance < MaxMoveDistance);
//
//	return bActionsUsed || bMoveDistanceUsed;
//}