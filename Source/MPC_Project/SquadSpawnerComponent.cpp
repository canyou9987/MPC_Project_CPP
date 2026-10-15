// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadSpawnerComponent.h"
#include "BaseCharacter.h"
#include "SquadManager.h"
#include "Engine/World.h"

// Sets default values for this component's properties
USquadSpawnerComponent::USquadSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USquadSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner가 SquadManager라면 자동 스폰 수행
	if (bAutoSpawnOnBeginPlay && UnitClassToSpawn)
	{
		if (ASquadManager* SquadMgr = Cast<ASquadManager>(GetOwner()))
		{
			SpawnSquadMembers(SquadMgr);
		}
	}
}




TArray<ABaseCharacter*> USquadSpawnerComponent::SpawnSquadMembers(ASquadManager* TargetSquadManager)
{
	TArray<ABaseCharacter*> SpawnedUnits;
	if (!UnitClassToSpawn || !GetWorld() || !TargetSquadManager) return SpawnedUnits;

	FVector OriginLocation = TargetSquadManager->GetActorLocation();
	FVector ForwardVector = TargetSquadManager->GetActorForwardVector();
	FVector RightVector = TargetSquadManager->GetActorRightVector();

	// 진형 기준 열(Columns) 수 연산
	int32 MaxPerRow = TargetSquadManager->MaxUnitsPerRow;
	float Spacing = TargetSquadManager->UnitSpacing;
	int32 Columns = (TargetSquadManager->FormationType == ESquadFormationType::Line) ? SpawnCount : MaxPerRow;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = TargetSquadManager;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		int32 Row = i / Columns;
		int32 Col = i % Columns;

		float RowOffset = -Row * Spacing;
		float ColOffset = (Col - (Columns - 1) * 0.5f) * Spacing;

		// 1. TargetSquadManager 기준 오프셋 위치 계산
		FVector Target2DPos = OriginLocation + (ForwardVector * RowOffset) + (RightVector * ColOffset);

		// 2. LineTrace로 정확한 지면 좌표 계산
		FVector FinalSpawnLocation = FindGroundLocation(Target2DPos);

		// 3. 동적 스폰
		ABaseCharacter* SpawnedUnit = GetWorld()->SpawnActor<ABaseCharacter>(
			UnitClassToSpawn,
			FinalSpawnLocation,
			TargetSquadManager->GetActorRotation(),
			SpawnParams
		);

		if (SpawnedUnit)
		{
			// 4. SquadManager에 유닛 자동 등록
			TargetSquadManager->RegisterMember(SpawnedUnit);

			if (!SpawnedUnit->GetController())
			{
				SpawnedUnit->SpawnDefaultController();
			}

			SpawnedUnits.Add(SpawnedUnit);


		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[SquadSpawnerComponent] %d has been danamic spanwnd!"), SpawnedUnits.Num());

	return SpawnedUnits;
}

FVector USquadSpawnerComponent::FindGroundLocation(const FVector& Target2DPos) const
{
	FVector TraceStart = Target2DPos + FVector(0.0f, 0.0f, 500.0f);
	FVector TraceEnd = Target2DPos - FVector(0.0f, 0.0f, 1000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	if (GetOwner())
	{
		QueryParams.AddIgnoredActor(GetOwner());
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// 유닛 캡슐 높이(HalfHeight ~90cm)를 감안해 지면 위로 보정
		return FVector(Target2DPos.X, Target2DPos.Y, HitResult.ImpactPoint.Z + 90.0f);
	}

	return Target2DPos;
}

