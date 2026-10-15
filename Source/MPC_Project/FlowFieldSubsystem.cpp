// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldSubsystem.h"
#include "DrawDebugHelpers.h"
#include <queue>

void UFlowFieldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 기본 50x50 그리드로 자동 초기화 (월드 원점 기준)
	GenerateGrid(FVector::ZeroVector, 50, 50, 50.0f);
}

void UFlowFieldSubsystem::GenerateGrid(FVector OriginLocation, int32 SizeX, int32 SizeY, float InNodeRadius)
{
	GridOrigin = OriginLocation;
	GridSizeX = SizeX;
	GridSizeY = SizeY;
	NodeRadius = InNodeRadius;

	float NodeDiameter = NodeRadius * 2.0f;
	float HalfWidth = (GridSizeX * NodeDiameter) * 0.5f;
	float HalfHeight = (GridSizeY * NodeDiameter) * 0.5f;

	Nodes.Empty();
	Nodes.AddDefaulted(GridSizeX * GridSizeY);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(NodeRadius - 5.0f, NodeRadius - 5.0f, 100.0f));

	for (int32 x = 0; x < GridSizeX; ++x)
	{
		for (int32 y = 0; y < GridSizeY; ++y)
		{
			int32 Index = x * GridSizeY + y;

			FVector WorldPos = GridOrigin + FVector(
				(x * NodeDiameter) + NodeRadius - HalfWidth,
				(y * NodeDiameter) + NodeRadius - HalfHeight,
				0.0f
			);

			Nodes[Index].WorldPosition = WorldPos;
			Nodes[Index].GridX = x;
			Nodes[Index].GridY = y;

			FHitResult HitResult;
			bool bHit = GetWorld()->SweepSingleByChannel(
				HitResult,
				WorldPos + FVector(0.0f, 0.0f, 100.0f),
				WorldPos + FVector(0.0f, 0.0f, 10.0f),
				FQuat::Identity,
				ECC_Visibility,
				BoxShape,
				QueryParams
			);

			bool bIsObstacle = false;

			if (bHit && HitResult.GetActor())
			{
				AActor* HitActor = HitResult.GetActor();
				FString ActorName = HitActor->GetName();

				bool bIsFloor = ActorName.Contains(TEXT("Floor")) || ActorName.Contains(TEXT("Landscape"));
				bool bIsCharacter = HitActor->IsA(APawn::StaticClass());

				if (!bIsFloor && !bIsCharacter)
				{
					bIsObstacle = true;
				}
			}

			Nodes[Index].Cost = bIsObstacle ? 255 : 1;
			Nodes[Index].bIsObstacle = bIsObstacle;
		}
	}

	// ★ 필수: BaseGridNodes에 복사하여 원본 지형 보관!
	BaseGridNodes = Nodes;

	UE_LOG(LogTemp, Warning, TEXT("[FlowFieldSubsystem] Cost Field Generated via SweepTest!"));
}


FFlowFieldNode* UFlowFieldSubsystem::GetNodeFromWorldPosition(const FVector& WorldPos)
{
	int32 Index = GetNodeIndexFromWorldPosition(WorldPos);
	if (Nodes.IsValidIndex(Index))
	{
		return &Nodes[Index];
	}
	return nullptr;
}

FFlowFieldNode* UFlowFieldSubsystem::GetNodeFromIndex(int32 X, int32 Y)
{
	// 전체 그리드 범위를 벗어나면 즉시 nullptr 반환
	if (X < 0 || X >= GridSizeX || Y < 0 || Y >= GridSizeY)
	{
		return nullptr;
	}

	int32 Index = X * GridSizeY + Y;
	if (Nodes.IsValidIndex(Index))
	{
		return &Nodes[Index];
	}
	return nullptr;
}

void UFlowFieldSubsystem::CalculateIntegrationField(const FVector& GoalWorldPosition)
{
	// 단일 지점 테스트용 호환 함수
	CalculateSquadFlowField(0, GoalWorldPosition, { GoalWorldPosition });
}

void UFlowFieldSubsystem::CalculateFlowField()
{
	int32 DirX[8] = { 0,  0, -1, 1, -1,  1, -1, 1 };
	int32 DirY[8] = { -1, 1,  0, 0, -1, -1,  1, 1 };

	for (FFlowFieldNode& Node : Nodes)
	{
		// [수정] 장애물 타일이더라도, 8방향 주변 중 '장애물이 아니고 BestCost가 가장 낮은 타일'을 향해 탈출 벡터를 제공!
		uint16 BestNeighborCost = 65535;
		FVector2D BestDirection = FVector2D::ZeroVector;

		for (int32 i = 0; i < 8; ++i)
		{
			FFlowFieldNode* Neighbor = GetNodeFromIndex(Node.GridX + DirX[i], Node.GridY + DirY[i]);
			if (!Neighbor || Neighbor->Cost == 255) continue; // 이웃도 장애물이면 패스

			if (Neighbor->BestCost < BestNeighborCost)
			{
				BestNeighborCost = Neighbor->BestCost;
				BestDirection = FVector2D(DirX[i], DirY[i]).GetSafeNormal();
			}
		}

		Node.UnitDirection = BestDirection;
	}

	UE_LOG(LogTemp, Warning, TEXT("[FlowFieldSubsystem] Flow Field Direction Vectors Generated!"));
}

void UFlowFieldSubsystem::CalculateLocalSquadFlowField(const FVector& TargetLocation, const TArray<FVector>& FormationOffsets, TArray<FVector>& OutUnitDirections)
{
	if (BaseGridNodes.Num() == 0)
	{
		GenerateGrid(GridWorldOrigin, GridSizeX, GridSizeY, NodeRadius);
	}

	int32 TotalNodes = GridSizeX * GridSizeY;

	// 1. 출력 방향 배열 크기 맞춤 및 초기화
	if (OutUnitDirections.Num() != TotalNodes)
	{
		OutUnitDirections.Init(FVector::ZeroVector, TotalNodes);
	}
	else
	{
		for (FVector& Dir : OutUnitDirections)
		{
			Dir = FVector::ZeroVector;
		}
	}

	// 2. Integration Field (누적 코스트 지도) 임시 연산용 배열
	TArray<uint16> IntegrationCosts;
	IntegrationCosts.Init(65535, TotalNodes);

	TQueue<int32> OpenQueue;

	// [수정됨] 단일 목적지(메인 타깃)만 Cost 0으로 큐에 삽입
	int32 MainTargetIndex = GetNodeIndexFromWorldPosition(TargetLocation);
	if (IsValidNodeIndex(MainTargetIndex) && !BaseGridNodes[MainTargetIndex].bIsObstacle)
	{
		IntegrationCosts[MainTargetIndex] = 0;
		OpenQueue.Enqueue(MainTargetIndex);
	}

	// 3. 다익스트라 / BFS 확장 연산 (8방향)
	int32 DirX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	int32 DirY[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
	uint16 DirCost[8] = { 10, 10, 10, 10, 14, 14, 14, 14 };

	while (!OpenQueue.IsEmpty())
	{
		int32 CurrentIndex;
		OpenQueue.Dequeue(CurrentIndex);

		int32 CurX = CurrentIndex / GridSizeY;
		int32 CurY = CurrentIndex % GridSizeY;

		for (int32 i = 0; i < 8; ++i)
		{
			int32 NextX = CurX + DirX[i];
			int32 NextY = CurY + DirY[i];

			if (NextX >= 0 && NextX < GridSizeX && NextY >= 0 && NextY < GridSizeY)
			{
				int32 NextIndex = NextX * GridSizeY + NextY;

				// 장애물이 아닌 노드만 확산
				if (IsValidNodeIndex(NextIndex) && !BaseGridNodes[NextIndex].bIsObstacle)
				{
					uint16 NewCost = IntegrationCosts[CurrentIndex] + DirCost[i] * BaseGridNodes[NextIndex].Cost;
					if (NewCost < IntegrationCosts[NextIndex])
					{
						IntegrationCosts[NextIndex] = NewCost;
						OpenQueue.Enqueue(NextIndex);
					}
				}
			}
		}
	}

	// 4. Flow Field (방향 벡터) 연산 -> OutUnitDirections에 저장
	for (int32 NodeIdx = 0; NodeIdx < TotalNodes; ++NodeIdx)
	{
		// 고립 구역 패스
		if (IntegrationCosts[NodeIdx] == 65535 && !BaseGridNodes[NodeIdx].bIsObstacle)
		{
			continue;
		}

		int32 CurX = NodeIdx / GridSizeY;
		int32 CurY = NodeIdx % GridSizeY;

		// ---------------------------------------------------------------------
		// [CASE A] 장애물(빨간색/코너) 타일인 경우 -> 가장 가까운 비장애물 타일로 강제 밀쳐내기 (탈출 벡터 복구!)
		// ---------------------------------------------------------------------
		if (BaseGridNodes[NodeIdx].bIsObstacle)
		{
			uint16 LowestValidCost = 65535;
			FVector2D PushOutDir = FVector2D::ZeroVector;

			for (int32 d = 0; d < 8; ++d)
			{
				int32 NeighborX = CurX + DirX[d];
				int32 NeighborY = CurY + DirY[d];

				if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
				{
					int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;

					// 장애물이 아닌 타일 중 BestCost가 가장 낮아 빠르게 탈출할 수 있는 방향
					if (!BaseGridNodes[NeighborIndex].bIsObstacle && IntegrationCosts[NeighborIndex] < LowestValidCost)
					{
						LowestValidCost = IntegrationCosts[NeighborIndex];
						PushOutDir = FVector2D(DirX[d], DirY[d]).GetSafeNormal();
					}
				}
			}

			// 만약 바로 옆 8타일에 비장애물이 없다면(깊은 장애물 내부), 아무 비장애물 방향이나 바라보게 처리
			if (PushOutDir.IsZero())
			{
				for (int32 d = 0; d < 8; ++d)
				{
					int32 NeighborX = CurX + DirX[d];
					int32 NeighborY = CurY + DirY[d];

					if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
					{
						int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;
						if (!BaseGridNodes[NeighborIndex].bIsObstacle)
						{
							PushOutDir = FVector2D(DirX[d], DirY[d]).GetSafeNormal();
							break;
						}
					}
				}
			}

			OutUnitDirections[NodeIdx] = FVector(PushOutDir.X, PushOutDir.Y, 0.0f);
			continue;
		}

		// ---------------------------------------------------------------------
		// [CASE B] 일반 평지 타일인 경우 -> BestCost가 가장 낮은 이웃 방향으로 이동
		// ---------------------------------------------------------------------
		uint16 BestNeighborCost = IntegrationCosts[NodeIdx];
		FVector BestDir = FVector::ZeroVector;

		for (int32 i = 0; i < 8; ++i)
		{
			int32 NextX = CurX + DirX[i];
			int32 NextY = CurY + DirY[i];

			if (NextX >= 0 && NextX < GridSizeX && NextY >= 0 && NextY < GridSizeY)
			{
				int32 NextIndex = NextX * GridSizeY + NextY;

				// 평지에서는 장애물 타일 안쪽으로 들어가지 않도록 방어
				if (BaseGridNodes[NextIndex].bIsObstacle) continue;

				if (IntegrationCosts[NextIndex] < BestNeighborCost)
				{
					BestNeighborCost = IntegrationCosts[NextIndex];
					BestDir = FVector(DirX[i], DirY[i], 0.0f).GetSafeNormal();
				}
			}
		}

		OutUnitDirections[NodeIdx] = BestDir;
	}

	// =========================================================
	// [디버그 시각화용 Nodes 배열 임시 업데이트]
	// =========================================================
	if (Nodes.Num() != TotalNodes)
	{
		Nodes = BaseGridNodes; // 기본 지형 복사
	}

	for (int32 NodeIdx = 0; NodeIdx < TotalNodes; ++NodeIdx)
	{
		Nodes[NodeIdx].BestCost = IntegrationCosts[NodeIdx];
		Nodes[NodeIdx].UnitDirection = FVector2D(OutUnitDirections[NodeIdx].X, OutUnitDirections[NodeIdx].Y);
	}
}

bool UFlowFieldSubsystem::IsObstacleAtWorldPosition(const FVector& WorldPos) const
{
	int32 Index = GetNodeIndexFromWorldPosition(WorldPos);
	if (IsValidNodeIndex(Index) && BaseGridNodes.IsValidIndex(Index))
	{
		return BaseGridNodes[Index].bIsObstacle;
	}
	return false;
}

FVector UFlowFieldSubsystem::GetNearestValidNodeLocation(const FVector& WorldPos) const
{
	int32 CenterIndex = GetNodeIndexFromWorldPosition(WorldPos);
	if (!IsValidNodeIndex(CenterIndex) || !BaseGridNodes.IsValidIndex(CenterIndex))
	{
		return WorldPos;
	}

	// 이미 평지(비장애물)라면 원래 좌표 그대로 반환
	if (!BaseGridNodes[CenterIndex].bIsObstacle)
	{
		return WorldPos;
	}

	int32 CurX = CenterIndex / GridSizeY;
	int32 CurY = CenterIndex % GridSizeY;

	float BestDistSq = FLT_MAX;
	FVector BestLocation = WorldPos;

	// 중심점으로부터 반경 1~5타일 범위를 넓혀가며 가장 가까운 평지 타일 탐색
	for (int32 Radius = 1; Radius <= 5; ++Radius)
	{
		bool bFoundInRadius = false;

		for (int32 dx = -Radius; dx <= Radius; ++dx)
		{
			for (int32 dy = -Radius; dy <= Radius; ++dy)
			{
				// 테두리 노드만 검사 (효율화)
				if (FMath::Abs(dx) != Radius && FMath::Abs(dy) != Radius) continue;

				int32 CheckX = CurX + dx;
				int32 CheckY = CurY + dy;

				if (CheckX >= 0 && CheckX < GridSizeX && CheckY >= 0 && CheckY < GridSizeY)
				{
					int32 CheckIndex = CheckX * GridSizeY + CheckY;
					if (BaseGridNodes.IsValidIndex(CheckIndex) && !BaseGridNodes[CheckIndex].bIsObstacle)
					{
						// =========================================================
						// [추가] 해당 평지 타일 주변 8방향에 장애물이 없는 '안전한 타일'인지 검사
						// =========================================================
						bool bIsSafeTile = true;
						for (int32 nx = -1; nx <= 1; ++nx)
						{
							for (int32 ny = -1; ny <= 1; ++ny)
							{
								int32 NearX = CheckX + nx;
								int32 NearY = CheckY + ny;
								if (NearX >= 0 && NearX < GridSizeX && NearY >= 0 && NearY < GridSizeY)
								{
									int32 NearIndex = NearX * GridSizeY + NearY;
									if (BaseGridNodes[NearIndex].bIsObstacle)
									{
										bIsSafeTile = false; // 장애물과 바짝 붙은 타일은 제껴버림!
										break;
									}
								}
							}
							if (!bIsSafeTile) break;
						}

						// 장애물과 떨어진 완전 안전한 평지만 지정석 후보로 채택
						if (bIsSafeTile)
						{
							float DistSq = FVector::DistSquared2D(WorldPos, BaseGridNodes[CheckIndex].WorldPosition);
							if (DistSq < BestDistSq)
							{
								BestDistSq = DistSq;
								BestLocation = BaseGridNodes[CheckIndex].WorldPosition;
								bFoundInRadius = true;
							}
						}
					}
				}
			}
		}

		// 가장 가까운 반경에서 평지를 찾았으면 탐색 중단
		if (bFoundInRadius)
		{
			break;
		}
	}

	return BestLocation;
}

int32 UFlowFieldSubsystem::GetNodeIndexFromWorldPosition(const FVector& WorldPosition) const
{
	float NodeDiameter = NodeRadius * 2.0f;
	float TotalWidth = GridSizeX * NodeDiameter;
	float TotalHeight = GridSizeY * NodeDiameter;

	FVector GridMinCorner = GridOrigin - FVector(TotalWidth * 0.5f, TotalHeight * 0.5f, 0.0f);
	FVector LocalPos = WorldPosition - GridMinCorner;

	int32 x = FMath::FloorToInt(LocalPos.X / NodeDiameter);
	int32 y = FMath::FloorToInt(LocalPos.Y / NodeDiameter);

	x = FMath::Clamp(x, 0, GridSizeX - 1);
	y = FMath::Clamp(y, 0, GridSizeY - 1);

	return x * GridSizeY + y;
}

bool UFlowFieldSubsystem::IsValidNodeIndex(int32 Index) const
{
	return Index >= 0 && Index < (GridSizeX * GridSizeY);
}

void UFlowFieldSubsystem::CalculateSquadFlowField(int32 SquadID, const FVector& MainTargetLocation, const TArray<FVector>& UnitTargetLocations)
{
	if (BaseGridNodes.Num() == 0)
	{
		// 기본 그리드가 생성되지 않았다면 먼저 생성
		GenerateGrid(GridWorldOrigin, GridSizeX, GridSizeY, NodeRadius);
	}

	// 1. 해당 SquadID 전용 레이어 생성 또는 기존 레이어 초기화
	FFlowFieldLayer& Layer = SquadLayers.FindOrAdd(SquadID);
	Layer.SquadID = SquadID;
	Layer.MainTargetLocation = MainTargetLocation;
	Layer.UnitTargetLocations = UnitTargetLocations;

	// 원본 지형 장애물 정보 복사 (BaseGridNodes)
	Layer.Nodes = BaseGridNodes;

	// -------------------------------------------------------------------------
	// 2. Integration Field (다익스트라 누적 코스트 연산)
	// -------------------------------------------------------------------------
	TQueue<int32> OpenQueue;

	// 부대원 N명의 개별 오프셋 목표지점 노드들의 BestCost를 0으로 설정 후 큐에 삽입
	for (const FVector& TargetPos : UnitTargetLocations)
	{
		int32 TargetIndex = GetNodeIndexFromWorldPosition(TargetPos);
		if (IsValidNodeIndex(TargetIndex) && !Layer.Nodes[TargetIndex].bIsObstacle)
		{
			Layer.Nodes[TargetIndex].BestCost = 0;
			OpenQueue.Enqueue(TargetIndex);
		}
	}

	// 큐가 비어있다면 메인 타깃 하나라도 시작점으로 설정
	if (OpenQueue.IsEmpty())
	{
		int32 MainTargetIndex = GetNodeIndexFromWorldPosition(MainTargetLocation);
		if (IsValidNodeIndex(MainTargetIndex))
		{
			Layer.Nodes[MainTargetIndex].BestCost = 0;
			OpenQueue.Enqueue(MainTargetIndex);
		}
	}

	// 8방향 탐색용 변수 (상, 하, 좌, 우, 대각선 4개)
	int32 DirX[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	int32 DirY[] = { 1, -1, 0, 0, 1, 1, -1, -1 };
	uint16 DirCost[] = { 10, 10, 10, 10, 14, 14, 14, 14 }; // 직진 10, 대각선 14

	// 다익스트라 확산 연산 (BFS)
	while (!OpenQueue.IsEmpty())
	{
		int32 CurrentIndex;
		OpenQueue.Dequeue(CurrentIndex);

		FFlowFieldNode& CurrentNode = Layer.Nodes[CurrentIndex];

		for (int32 i = 0; i < 8; ++i)
		{
			int32 NeighborX = CurrentNode.GridX + DirX[i];
			int32 NeighborY = CurrentNode.GridY + DirY[i];

			if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
			{
				int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;
				FFlowFieldNode& NeighborNode = Layer.Nodes[NeighborIndex];

				// 장애물 타일(Cost == 255)은 탐색에서 제외
				if (NeighborNode.bIsObstacle || NeighborNode.Cost == 255)
				{
					continue;
				}

				uint16 NewBestCost = CurrentNode.BestCost + DirCost[i] * NeighborNode.Cost;
				if (NewBestCost < NeighborNode.BestCost)
				{
					NeighborNode.BestCost = NewBestCost;
					OpenQueue.Enqueue(NeighborIndex);
				}
			}
		}
	}

	// -------------------------------------------------------------------------
	// 3. Flow Field (8방향 벡터 지도 연산 & 코너/장애물 완전 탈출 로직)
	// -------------------------------------------------------------------------
	for (int32 i = 0; i < Layer.Nodes.Num(); ++i)
	{
		FFlowFieldNode& Node = Layer.Nodes[i];

		// 완전히 도달할 수 없는 고립 구역 패스
		if (Node.BestCost == 65535 && !Node.bIsObstacle)
		{
			Node.UnitDirection = FVector2D::ZeroVector;
			continue;
		}

		// ---------------------------------------------------------------------
		// [CASE A] 장애물(빨간색/코너) 타일인 경우 -> 가장 가까운 비장애물 타일로 강제 밀쳐내기
		// ---------------------------------------------------------------------
		if (Node.bIsObstacle)
		{
			uint16 LowestValidCost = 65535;
			FVector2D PushOutDir = FVector2D::ZeroVector;

			for (int32 d = 0; d < 8; ++d)
			{
				int32 NeighborX = Node.GridX + DirX[d];
				int32 NeighborY = Node.GridY + DirY[d];

				if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
				{
					int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;
					const FFlowFieldNode& NeighborNode = Layer.Nodes[NeighborIndex];

					// 장애물이 아닌 타일 중 BestCost가 가장 낮아 빠르게 탈출할 수 있는 방향 선택
					if (!NeighborNode.bIsObstacle && NeighborNode.BestCost < LowestValidCost)
					{
						LowestValidCost = NeighborNode.BestCost;
						PushOutDir = FVector2D(DirX[d], DirY[d]).GetSafeNormal();
					}
				}
			}

			// 만약 바로 옆 8타일에 비장애물이 없다면(깊은 장애물 내부), 아무 비장애물 방향이나 바라보게 처리
			if (PushOutDir.IsZero())
			{
				for (int32 d = 0; d < 8; ++d)
				{
					int32 NeighborX = Node.GridX + DirX[d];
					int32 NeighborY = Node.GridY + DirY[d];

					if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
					{
						int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;
						if (!Layer.Nodes[NeighborIndex].bIsObstacle)
						{
							PushOutDir = FVector2D(DirX[d], DirY[d]).GetSafeNormal();
							break;
						}
					}
				}
			}

			Node.UnitDirection = PushOutDir;
			continue; // 장애물 타일 처리 완료 후 다음 타일로 이동
		}

		// ---------------------------------------------------------------------
		// [CASE B] 일반 평지 타일인 경우 -> BestCost가 가장 낮은 이웃 방향으로 이동
		// ---------------------------------------------------------------------
		uint16 BestNeighborCost = Node.BestCost;
		FVector2D BestDirection = FVector2D::ZeroVector;

		for (int32 d = 0; d < 8; ++d)
		{
			int32 NeighborX = Node.GridX + DirX[d];
			int32 NeighborY = Node.GridY + DirY[d];

			if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
			{
				int32 NeighborIndex = NeighborX * GridSizeY + NeighborY;
				const FFlowFieldNode& NeighborNode = Layer.Nodes[NeighborIndex];

				// 평지에서는 장애물 타일 안쪽으로 들어가려고 하지 않도록 예외 처리
				if (NeighborNode.bIsObstacle)
				{
					continue;
				}

				if (NeighborNode.BestCost < BestNeighborCost)
				{
					BestNeighborCost = NeighborNode.BestCost;
					BestDirection = FVector2D(DirX[d], DirY[d]).GetSafeNormal();
				}
			}
		}

		Node.UnitDirection = BestDirection;
	}

	// 디버그 시각화용 노드 배열 업데이트
	Nodes = Layer.Nodes;
}

FVector UFlowFieldSubsystem::GetUnitDirectionForSquad(int32 SquadID, const FVector& WorldPosition) const
{
	const FFlowFieldLayer* Layer = SquadLayers.Find(SquadID);
	if (!Layer)
	{
		return FVector::ZeroVector;
	}

	int32 NodeIndex = GetNodeIndexFromWorldPosition(WorldPosition);
	if (IsValidNodeIndex(NodeIndex) && Layer->Nodes.IsValidIndex(NodeIndex))
	{
		const FVector2D& Dir2D = Layer->Nodes[NodeIndex].UnitDirection;
		return FVector(Dir2D.X, Dir2D.Y, 0.0f);
	}

	return FVector::ZeroVector;
}

uint16 UFlowFieldSubsystem::GetBestCostForSquad(int32 SquadID, const FVector& WorldPosition) const
{
	const FFlowFieldLayer* Layer = SquadLayers.Find(SquadID);
	if (!Layer)
	{
		return 65535;
	}

	int32 NodeIndex = GetNodeIndexFromWorldPosition(WorldPosition);
	if (IsValidNodeIndex(NodeIndex) && Layer->Nodes.IsValidIndex(NodeIndex))
	{
		return Layer->Nodes[NodeIndex].BestCost;
	}

	return 65535;
}

void UFlowFieldSubsystem::ClearSquadLayer(int32 SquadID)
{
	SquadLayers.Remove(SquadID);
}

TArray<FFlowFieldNode*> UFlowFieldSubsystem::GetNeighborNodes(int32 NodeX, int32 NodeY)
{
	TArray<FFlowFieldNode*> Neighbors;

	// 상, 하, 좌, 우 및 대각선 8방향 오프셋
	int32 DirX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	int32 DirY[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	for (int32 i = 0; i < 8; ++i)
	{
		FFlowFieldNode* Neighbor = GetNodeFromIndex(NodeX + DirX[i], NodeY + DirY[i]);
		if (Neighbor)
		{
			Neighbors.Add(Neighbor);
		}
	}

	return Neighbors;
}
