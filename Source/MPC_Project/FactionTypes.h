// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/UserDefinedEnum.h"
#include "FactionTypes.generated.h"


UENUM(BlueprintType)
enum class EFactionGroup : uint8
{
	PlayerParty     UMETA(DisplayName = "Player Party"),
	TownGuard       UMETA(DisplayName = "Town Guard"),
	GoblinTribe     UMETA(DisplayName = "Goblin Tribe"),
	Bandits         UMETA(DisplayName = "Bandits"),
	NeutralCreature UMETA(DisplayName = "Neutral Creature")
};

// 두 액터/진영 간의 상대적 관계(태도)
UENUM(BlueprintType)
enum class EFactionAttitude : uint8
{
	Friendly UMETA(DisplayName = "Friendly"), // 아군 (도움/힐 대상)
	Neutral  UMETA(DisplayName = "Neutral"),  // 중립 (선제공격 안함)
	Hostile  UMETA(DisplayName = "Hostile")   // 적대 (선제공격 및 타깃 대상)
};
