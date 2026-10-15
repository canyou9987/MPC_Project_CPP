// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldDebugVisualizer.h"
#include "FlowFieldSubsystem.h"
#include "DrawDebugHelpers.h"

// Sets default values
AFlowFieldDebugVisualizer::AFlowFieldDebugVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AFlowFieldDebugVisualizer::BeginPlay()
{
	Super::BeginPlay();

	// 모든 액터의 BeginPlay가 끝나 콜리전이 완전히 세팅된 시점에 그리드 재생성!
	if (UFlowFieldSubsystem* FlowSys = GetWorld()->GetSubsystem<UFlowFieldSubsystem>())
	{
		FlowSys->GenerateGrid(FVector::ZeroVector, 50, 50, 50.0f);
	}
}

// Called every frame
void AFlowFieldDebugVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DebugMode == EFlowFieldDebugMode::Off) return;

	if (UFlowFieldSubsystem* FlowSys = GetWorld()->GetSubsystem<UFlowFieldSubsystem>())
	{
		const TArray<FFlowFieldNode>& Nodes = FlowSys->GetNodes();
		float Radius = FlowSys->GetNodeRadius();

		for (const FFlowFieldNode& Node : Nodes)
		{
			// --- Mode 1: Cost Field (정적 지형 장애물 확인) ---
			if (DebugMode == EFlowFieldDebugMode::CostField)
			{
				FColor BoxColor = (Node.Cost == 255) ? FColor::Red : FColor::Green;
				DrawDebugBox(
					GetWorld(),
					Node.WorldPosition,
					FVector(Radius - 2.0f, Radius - 2.0f, 2.0f),
					BoxColor,
					false,
					-1.0f,
					0,
					1.0f
				);
			}
			// --- Mode 2: Integration Field (목적지 기준 누적 거리 그라데이션) ---
			else if (DebugMode == EFlowFieldDebugMode::IntegrationField)
			{
				if (Node.Cost == 255)
				{
					// 장애물은 빨간색으로 경계 표시
					DrawDebugBox(GetWorld(), Node.WorldPosition, FVector(Radius - 2.0f, Radius - 2.0f, 2.0f), FColor::Red, false, -1.0f, 0, 1.0f);
				}
				else if (Node.BestCost != 65535)
				{
					// BestCost에 맞춰 파란색(목적지 근처) ~ 노란색/적색(멀어짐) 그라데이션
					uint8 Intensity = FMath::Clamp((int32)Node.BestCost / 2, 0, 255);
					FColor GradientColor = FColor(Intensity, 255 - Intensity, 255 - (Intensity / 2));

					DrawDebugBox(
						GetWorld(),
						Node.WorldPosition,
						FVector(Radius - 2.0f, Radius - 2.0f, 2.0f),
						GradientColor,
						false,
						-1.0f,
						0,
						1.0f
					);
				}
			}
			// --- Mode 3: Flow Field (방향 화살표 지도) ---
			else if (DebugMode == EFlowFieldDebugMode::FlowField)
			{
				// Step 3에서 구현할 방향 화살표(Vector) 그리기 영역
				// 장애물이 아니고 이동 방향 벡터가 존재하는 노드만 화살표 렌더링
				if (Node.Cost != 255 && !Node.UnitDirection.IsNearlyZero())
				{
					FVector LineStart = Node.WorldPosition + FVector(0.0f, 0.0f, 10.0f);
					FVector Direction3D = FVector(Node.UnitDirection.X, Node.UnitDirection.Y, 0.0f);
					FVector LineEnd = LineStart + (Direction3D * (Radius * 0.6f)); // 타일 크기에 맞게 화살표 길이 조절

					DrawDebugDirectionalArrow(
						GetWorld(),
						LineStart,
						LineEnd,
						15.0f,          // 화살표 머리 크기
						FColor::Cyan,   // 시원한 하늘색 화살표
						false,
						-1.0f,
						0,
						1.5f            // 두께
					);
				}
				else if (Node.Cost == 255)
				{
					// 장애물은 빨간 상자로 표시
					DrawDebugBox(GetWorld(), Node.WorldPosition, FVector(Radius - 2.0f, Radius - 2.0f, 2.0f), FColor::Red, false, -1.0f, 0, 1.0f);
				}
			}
		}
	}
}

