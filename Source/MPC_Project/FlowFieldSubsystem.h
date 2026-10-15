// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlowFieldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFlowFieldNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	int32 GridX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	int32 GridY = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	uint8 Cost = 1; // 1: 기본 지형, 255: 장애물

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	uint16 BestCost = 65535; // 다익스트라 누적 코스트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	FVector2D UnitDirection = FVector2D::ZeroVector; // 방향 벡터

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField")
	bool bIsObstacle = false;
};

// -------------------------------------------------------------------------
// 부대 전용 유동장 레이어 구조체 (SquadID 기반 독립 레이어)
// -------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FFlowFieldLayer
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SquadID = -1;

	UPROPERTY()
	FVector MainTargetLocation = FVector::ZeroVector;

	// 부대원 N명의 개별 오프셋 목표 좌표들
	UPROPERTY()
	TArray<FVector> UnitTargetLocations;

	// 해당 부대 전용 노드/방향 지도
	UPROPERTY()
	TArray<FFlowFieldNode> Nodes;
};

UCLASS()
class MPC_PROJECT_API UFlowFieldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void GenerateGrid(FVector OriginLocation, int32 SizeX, int32 SizeY, float InNodeRadius);

	// 디버그 시각화용 데이터 획득 함수
	const TArray<FFlowFieldNode>& GetNodes() const { return Nodes; }
	int32 GetGridSizeX() const { return GridSizeX; }
	int32 GetGridSizeY() const { return GridSizeY; }
	float GetNodeRadius() const { return NodeRadius; }

	// 좌표 변환 헬퍼
	FFlowFieldNode* GetNodeFromWorldPosition(const FVector& WorldPos);

	// [추가] X, Y 그리드 인덱스로 노드 안전하게 가져오기
	FFlowFieldNode* GetNodeFromIndex(int32 X, int32 Y);

public:

	// =========================================================================
	// 부대별 멀티 레이어 유동장 연산 및 조회 API
	// =========================================================================

	/** 특정 부대(SquadID) 전용 유동장(Integration & Flow Field) 연산 요청 */
	void CalculateSquadFlowField(int32 SquadID, const FVector& MainTargetLocation, const TArray<FVector>& UnitTargetLocations);

	/** 특정 부대의 특정 월드 좌표 노드 방향 벡터 반환 */
	FVector GetUnitDirectionForSquad(int32 SquadID, const FVector& WorldPosition) const;

	/** 특정 부대의 특정 월드 좌표 노드 BestCost 반환 (목적지 도달/정지 판정용) */
	uint16 GetBestCostForSquad(int32 SquadID, const FVector& WorldPosition) const;

	/** 부대 이동 완료 시 해당 부대의 유동장 레이어 메모리 제거 */
	void ClearSquadLayer(int32 SquadID);

	// 우클릭한 월드 좌표를 기준으로 Integration Field 연산 실행
	void CalculateIntegrationField(const FVector& GoalWorldPosition);

	// Flow Field 방향 벡터 연산 함수
	void CalculateFlowField();

	void CalculateLocalSquadFlowField(
		const FVector& TargetLocation,
		const TArray<FVector>& FormationOffsets,
		TArray<FVector>& OutUnitDirections
	);

	/** 특정 월드 좌표가 장애물(bIsObstacle) 타일인지 확인 */
	bool IsObstacleAtWorldPosition(const FVector& WorldPos) const;

	/** 특정 월드 좌표가 장애물이면 가장 가까운 비장애물 타일의 월드 좌표 반환 */
	FVector GetNearestValidNodeLocation(const FVector& WorldPos) const;

	// =========================================================================
	// 헬퍼 함수
	// =========================================================================
	int32 GetNodeIndexFromWorldPosition(const FVector& WorldPosition) const;
	bool IsValidNodeIndex(int32 Index) const;

public:
	// 그리드 설정 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField|Grid")
	int32 GridSizeX = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField|Grid")
	int32 GridSizeY = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField|Grid")
	float NodeRadius = 100.0f; // 노드 반지름 (타일 크기: 200x200cm)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowField|Grid")
	FVector GridWorldOrigin = FVector::ZeroVector;

	FVector GridOrigin = FVector::ZeroVector;

private:
	TArray<FFlowFieldNode> Nodes;

	// 기본 지형 장애물 정보가 담긴 베이스 노드 배열 (Cost Field 기준)
	UPROPERTY()
	TArray<FFlowFieldNode> BaseGridNodes;

	// SquadID를 Key로 가지는 부대별 독립 유동장 레이어 보관소!
	UPROPERTY()
	TMap<int32, FFlowFieldLayer> SquadLayers;

private:
	// 지정한 노드의 상/하/좌/우(4방향) 인접 노드들을 가져오는 헬퍼 함수
	TArray<FFlowFieldNode*> GetNeighborNodes(int32 NodeX, int32 NodeY);

};
