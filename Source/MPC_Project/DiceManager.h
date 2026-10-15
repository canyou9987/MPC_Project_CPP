// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <random>
#include "DiceManager.generated.h"

// 주사위 굴림 세부 결과를 담는 구조체 (로그 출력 및 UI 표시용)
USTRUCT(BlueprintType)
struct FD20RollResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RawRoll = 0; // 순수 주사위 눈금 (1~20)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Bonus = 0; // 스탯/BAB 수정치

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Total = 0; // 최종 합산값 (RawRoll + Bonus)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCriticalSuccess = false; // 내츄럴 20

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCriticalFailure = false; // 내츄럴 1
};

UCLASS()
class MPC_PROJECT_API UDiceManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// 커스텀 난수 생성기 초기화 (GameInstance나 서브시스템에서 필요 시 Seed 설정)
	static void InitializeSeed(uint32 Seed);

	// 범위 난수 생성 헬퍼 함수
	static int32 GetRandomRange(int32 Min, int32 Max);

	// d20 굴림 함수 (static 키워드로 인해 객체 생성 없이 직접 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Dice Engine")
	static FD20RollResult RollD20(int32 Bonus = 0);

	// 명중 여부 판정 함수
	UFUNCTION(BlueprintCallable, Category = "Dice Engine")
	static bool CheckHit(const FD20RollResult& RollResult, int32 TargetAC);

	// 데미지 주사위 굴림 함수 (예: 1d8 + Bonus)
	UFUNCTION(BlueprintCallable, Category = "Dice Engine")
	static int32 RollDamage(int32 DiceCount, int32 DiceSides, int32 Bonus = 0);

	// D&D 3.5e 내성 굴림 판정 (d20 + SaveBonus vs DC)
	UFUNCTION(BlueprintCallable, Category = "Dice Engine")
	static bool CheckSavingThrow(int32 SaveBonus, int32 DC, FD20RollResult& OutRollResult);

private:
	// C++11 Mersenne Twister 난수 생성기 엔진
	static std::mt19937 RandomEngine;
};
