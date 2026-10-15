// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTargetType.generated.h"

UENUM(BlueprintType)
enum class EActionTargetType : uint8
{
	None,         // 클릭 무효 / 허공
	Ground,       // 일반 지면 (NavMesh 이동)
	Enemy,        // 적 캐릭터 (공격)
	Friendly,     // 아군 / 동료 / NPC (선택 및 대화)
	Interactable, // 문, 레버, 닫힌 상자 (상호작용)
	Loot,         // 바닥 아이템, 시체, 열린 상자 (아이템 획득 UI)
	Unreachable   // NavMesh 외부 지형 (이동 불가 피드백)
};
