// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceManager.h"

// static 멤버 변수 정의 및 기본 시드(std::random_device) 초기화
std::mt19937 UDiceManager::RandomEngine(std::random_device{}());

void UDiceManager::InitializeSeed(uint32 Seed)
{
	RandomEngine.seed(Seed);
}

int32 UDiceManager::GetRandomRange(int32 Min, int32 Max)
{
	if (Min > Max) std::swap(Min, Max);

	// uniform_int_distribution을 사용해 균등 분포 난수 생성
	std::uniform_int_distribution<int32> Dist(Min, Max);
	return Dist(RandomEngine);
}

FD20RollResult UDiceManager::RollD20(int32 Bonus)	//	보너스를 가지고 주사위를 굴림
{
	FD20RollResult Result;	//	주사위 결과와 보너스를 저장할 

	// FMath::RandRange(1, 20) 대신 C++ mt19937 활용
	Result.RawRoll = GetRandomRange(1, 20);
	Result.Bonus = Bonus;
	Result.Total = Result.RawRoll + Bonus;

	if (Result.RawRoll == 20) Result.bIsCriticalSuccess = true;
	if (Result.RawRoll == 1) Result.bIsCriticalFailure = true;

	return Result;
}

bool UDiceManager::CheckHit(const FD20RollResult& RollResult, int32 TargetAC)
{
	// 내츄럴 20은 무조건 명중
	if (RollResult.bIsCriticalSuccess) return true;

	// 내츄럴 1은 무조건 빗나감
	if (RollResult.bIsCriticalFailure) return false;

	// 총점 vs AC 비교
	return RollResult.Total >= TargetAC;
}

int32 UDiceManager::RollDamage(int32 DiceCount, int32 DiceSides, int32 Bonus)
{
	int32 TotalDamage = 0;

	for (int32 i = 0; i < DiceCount; ++i)
	{
		TotalDamage += GetRandomRange(1, DiceSides);
	}

	// 최소 데미지는 1로 보정
	return FMath::Max(1, TotalDamage + Bonus);
}

bool UDiceManager::CheckSavingThrow(int32 SaveBonus, int32 DC, FD20RollResult& OutRollResult)
{
	OutRollResult = RollD20(SaveBonus);

	// Natural 20 / 1 예외 처리
	if (OutRollResult.bIsCriticalSuccess) return true;  // 내성 성공
	if (OutRollResult.bIsCriticalFailure) return false; // 내성 실패

	return OutRollResult.Total >= DC;
}
