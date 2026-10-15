// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnitRegistrySubsystem.generated.h"

/**
 * 월드 내 선택 가능한 유닛(ISelectableInterface)들을 중앙에서 등록/관리하는 경량 서브시스템.
 * 레벨 시작 시 자동 생성되고 레벨 종료 시 자동 파괴됨.
 */

UCLASS()
class MPC_PROJECT_API UUnitRegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 유닛 등록 (BeginPlay에서 호출)
	void RegisterUnit(AActor* UnitToRegister);

	// 유닛 해제 (EndPlay/사망 시 호출)
	void UnregisterUnit(AActor* UnitToUnregister);

	// 등록된 전체 유닛 명단 반환
	const TArray<TWeakObjectPtr<AActor>>& GetSelectableUnits() const { return SelectableUnits; }

private:
	// 포인터 유효성을 안전하게 보장하는 TWeakObjectPtr 기반 유닛 레지스트리
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SelectableUnits;
};
