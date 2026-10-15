// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SquadSpawnerComponent.generated.h"

class ABaseCharacter;
class ASquadManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API USquadSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USquadSpawnerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// 지정된 위치 기준 지면 트레이스 동적 스폰 실행
	UFUNCTION(BlueprintCallable, Category = "Squad|Spawn")
	TArray<ABaseCharacter*> SpawnSquadMembers(ASquadManager* TargetSquadManager);
		
public:
	// 스폰할 캐릭터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Spawn")
	TSubclassOf<ABaseCharacter> UnitClassToSpawn;

	// 동적 스폰할 유닛 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Spawn")
	int32 SpawnCount = 20;

	// 게임 시작 시 자동 스폰 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Spawn")
	bool bAutoSpawnOnBeginPlay = true;

private:
	// 지면 바닥 Z값 구하기용 LineTrace 헬퍼
	FVector FindGroundLocation(const FVector& Target2DPos) const;
};
