// Fill out your copyright notice in the Description page of Project Settings.


#include "FactionSubsystem.h"
#include "FactionInterface.h"

EFactionAttitude UFactionSubsystem::GetAttitude(AActor* SourceActor, AActor* TargetActor) const
{
	if (!SourceActor || !TargetActor || SourceActor == TargetActor)
	{
		return EFactionAttitude::Neutral;
	}

	EFactionGroup SourceGroup = GetActorFaction(SourceActor);
	EFactionGroup TargetGroup = GetActorFaction(TargetActor);

	return GetFactionAttitude(SourceGroup, TargetGroup);
}

EFactionAttitude UFactionSubsystem::GetFactionAttitude(EFactionGroup SourceGroup, EFactionGroup TargetGroup) const
{
	// 1. 같은 진영은 무조건 아군
	if (SourceGroup == TargetGroup)
	{
		return EFactionAttitude::Friendly;
	}

	// 2. 진영 간 관계 매트릭스 (세력표)
	switch (SourceGroup)
	{
	case EFactionGroup::PlayerParty:
		if (TargetGroup == EFactionGroup::TownGuard) return EFactionAttitude::Friendly;
		if (TargetGroup == EFactionGroup::GoblinTribe || TargetGroup == EFactionGroup::Bandits) return EFactionAttitude::Hostile;
		break;

	case EFactionGroup::GoblinTribe:
		// 고블린은 플레이어와 산적 모두에게 적대적
		if (TargetGroup == EFactionGroup::PlayerParty || TargetGroup == EFactionGroup::Bandits) return EFactionAttitude::Hostile;
		break;

	case EFactionGroup::TownGuard:
		// 경비대는 플레이어와는 아군/중립, 산적/고블린과는 적대
		if (TargetGroup == EFactionGroup::Bandits || TargetGroup == EFactionGroup::GoblinTribe) return EFactionAttitude::Hostile;
		break;

	case EFactionGroup::Bandits:
		// 산적은 마을경비대, 플레이어, 고블린 모두에게 적대적
		if (TargetGroup == EFactionGroup::PlayerParty || TargetGroup == EFactionGroup::TownGuard || TargetGroup == EFactionGroup::GoblinTribe) return EFactionAttitude::Hostile;
		break;

	default:
		break;
	}

	return EFactionAttitude::Neutral;
}

EFactionGroup UFactionSubsystem::GetActorFaction(AActor* TargetActor) const
{
	if (!TargetActor) return EFactionGroup::NeutralCreature;

	// IFactionInterface를 상속받은 액터라면 해당 팩션 반환
	if (IFactionInterface* FactionActor = Cast<IFactionInterface>(TargetActor))
	{
		return FactionActor->GetFactionGroup();
	}

	return EFactionGroup::NeutralCreature;
}
