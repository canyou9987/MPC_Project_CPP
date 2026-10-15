// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadManager.h"
#include "BaseCharacter.h"
#include "FlowFieldSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "SquadSpawnerComponent.h"

//static int32 GlobalSquadIDCounter = 1;

// Sets default values
ASquadManager::ASquadManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 액터 중심을 잡을 루트 컴포넌트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 스포너 컴포넌트 생성 및 부착
	SpawnerComponent = CreateDefaultSubobject<USquadSpawnerComponent>(TEXT("SpawnerComponent"));

	// 생성 시마다 고유 ID 자동 부여 (필요 시 에디터 수동 지정값도 허용 가능)
	//SquadID = GlobalSquadIDCounter++;

	// 기본 50x50 그리드로 Local FlowField 초기화
	LocalFlowField.Initialize(50, 50, 100.0f, FVector::ZeroVector);
}

// Called when the game starts or when spawned
void ASquadManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 에디터 패널에서 미리 등록한 부대원들에게 '내가 너의 부대장이다' 확정 명시
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member)
		{
			Member->SetSquadManager(this);
		}
	}
}

// Called every frame
void ASquadManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동 명령 진행 중일 때 정지 여부 체크
	if (bHasActiveMoveCommand)
	{
		bool bAllMembersStopped = true;
		for (const ABaseCharacter* Member : SquadMembers)
		{
			if (Member && Member->IsMovingWithFlowField())
			{
				bAllMembersStopped = false;
				break;
			}
		}

		if (bAllMembersStopped)
		{
			bHasActiveMoveCommand = false;
			// 더 이상 전역 Subsystem의 ClearSquadLayer를 호출하지 않고 Local 변수만 초기화
			LocalFlowField.UnitDirections.Init(FVector::ZeroVector, LocalFlowField.GridSizeX * LocalFlowField.GridSizeY);
		}
	}
}

void ASquadManager::RegisterMember(ABaseCharacter* NewMember)
{
	if (!NewMember) return;

	if (!SquadMembers.Contains(NewMember))
	{
		SquadMembers.Add(NewMember);
	}

	// 배열 포함 여부와 상관없이 부대장 연결은 확실하게 갱신
	NewMember->SetSquadManager(this);
}

void ASquadManager::UnregisterMember(ABaseCharacter* MemberToUnregister)
{
	if (!MemberToUnregister) return;

	SquadMembers.Remove(MemberToUnregister);
	MemberToUnregister->SetSquadManager(nullptr);
}

// -------------------------------------------------------------------------
// 선택/이동 명령 일괄 위임 (ISelectableInterface)
// -------------------------------------------------------------------------

void ASquadManager::SetSelected(bool bSelected)
{
	// 이미 같은 상태라면 중복 실행 방지 (재귀 루프 차단)
	if (bIsSquadSelected == bSelected) return;

	bIsSquadSelected = bSelected;

	for (ABaseCharacter* Member : SquadMembers)
	{
		// Member가 Valid하고, 혹시라도 자기 자신이 들어간 경우 제외
		if (Member && Member != Cast<ABaseCharacter>(this))
		{
			Member->SetSelected(bSelected);
		}
	}
}

void ASquadManager::StartFlowFieldMovement()
{
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member)
		{
			Member->StartFlowFieldMovement();
		}
	}
}

void ASquadManager::StopFlowFieldMovement()
{
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member)
		{
			Member->StopFlowFieldMovement();
		}
	}
}

void ASquadManager::CommandTargetAction(AActor* InTargetActor, EActionTargetType TargetType)
{
	// 부대원 전체에게 일괄 타깃 명령 전달 (공격, 상호작용 등)
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member)
		{
			// Member가 ISelectableInterface 가상 함수를 상속받아 오버라이딩했으므로 직접 호출!
			Member->CommandTargetAction(InTargetActor, TargetType);
		}
	}
}

void ASquadManager::CommandTargetAction(EActionTargetType TargetType, const FVector& TargetLocation)
{
	if (TargetType == EActionTargetType::Ground)
	{
		// 1. 메인 목표 지점 저장
		CurrentMainTargetLocation = TargetLocation;

		// 2. 클릭 위치 기준 부대원들의 '최종 진형 좌표' 계산
		TArray<FVector> UnitTargetLocations = CalculateFormationOffsets(TargetLocation, GetActorForwardVector());

		// 3. 유닛별 최종 목표 위치 매핑 (Dictionary 저장)
		MemberTargetLocations.Empty();
		for (int32 i = 0; i < SquadMembers.Num() && i < UnitTargetLocations.Num(); ++i)
		{
			if (SquadMembers[i])
			{
				MemberTargetLocations.Add(SquadMembers[i], UnitTargetLocations[i]);
			}
		}

		// 4. 단일 목표 지점(TargetLocation)으로 FlowField 연산 요청
		CalculateLocalFlowField(TargetLocation, UnitTargetLocations);

		// 5. 부원들에게 이동 신호 전달
		for (ABaseCharacter* Member : SquadMembers)
		{
			if (Member)
			{
				Member->StartFlowFieldMovement();
			}
		}
		bHasActiveMoveCommand = true;
	}
}

void ASquadManager::ToggleHoldPosition()
{
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member && Member->Implements<USelectableInterface>())
		{
			Member->ToggleHoldPosition();
		}
	}
}

void ASquadManager::ToggleAutoCombat()
{
	for (ABaseCharacter* Member : SquadMembers)
	{
		if (Member && Member->Implements<USelectableInterface>())
		{
			Member->ToggleAutoCombat();
		}
	}
}

TArray<FVector> ASquadManager::CalculateFormationOffsets(const FVector& TargetLocation, const FVector& MovementDirection)
{
	TArray<FVector> OutLocations;
	const int32 NumMembers = SquadMembers.Num();
	if (NumMembers == 0) return OutLocations;

	OutLocations.Reserve(NumMembers);

	// 부대가 바라볼 회전각 (Z축 평면 회전)
	FRotator TargetRotation = MovementDirection.IsNearlyZero() ? GetActorRotation() : MovementDirection.Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	FVector ForwardVector = TargetRotation.Vector();
	FVector RightVector = FRotationMatrix(TargetRotation).GetScaledAxis(EAxis::Y);

	int32 Columns = MaxUnitsPerRow;
	if (FormationType == ESquadFormationType::Line)
	{
		Columns = NumMembers; // 횡대: 한 줄로 펼침
	}
	else if (FormationType == ESquadFormationType::Column)
	{
		Columns = 1; // 종대: 1열 종대
	}

	for (int32 i = 0; i < NumMembers; ++i)
	{
		int32 Row = i / Columns;    // 줄 (0, 1, 2...)
		int32 Col = i % Columns;    // 열 (0, 1, 2...)

		// 중심점(Pivot) 기준 좌우/전후 배치 거리
		float RowOffset = -Row * UnitSpacing;
		float ColOffset = (Col - (Columns - 1) * 0.5f) * UnitSpacing;

		// 최종 3D 위치 좌표 계산
		FVector UnitTargetWorldPos = TargetLocation + (ForwardVector * RowOffset) + (RightVector * ColOffset);

		// =========================================================
		// [추가] 지정석이 장애물 내부면 가장 가까운 평지 타일로 보정
		// =========================================================
		if (UFlowFieldSubsystem* FlowSys = GetWorld()->GetSubsystem<UFlowFieldSubsystem>())
		{
			if (FlowSys->IsObstacleAtWorldPosition(UnitTargetWorldPos))
			{
				UnitTargetWorldPos = FlowSys->GetNearestValidNodeLocation(UnitTargetWorldPos);
			}
		}

		OutLocations.Add(UnitTargetWorldPos);
	}

	return OutLocations;
}

FVector ASquadManager::GetMoveDirectionForUnit(const FVector& UnitLocation) const
{
	// 1. Subsystem의 정확한 공식으로 인덱스를 받아옵니다.
	if (UFlowFieldSubsystem* FlowSys = GetWorld()->GetSubsystem<UFlowFieldSubsystem>())
	{
		int32 NodeIndex = FlowSys->GetNodeIndexFromWorldPosition(UnitLocation);

		// 2. 해당 인덱스가 SquadManager가 가진 배열 범위 내에 있는지 확인 후 반환
		if (LocalFlowField.UnitDirections.IsValidIndex(NodeIndex))
		{
			return LocalFlowField.UnitDirections[NodeIndex];
		}
	}

	return FVector::ZeroVector;
}

int32 ASquadManager::WorldToGridIndex(const FVector& WorldLocation) const
{
	FVector LocalPos = WorldLocation - LocalFlowField.GridOrigin;
	int32 X = FMath::FloorToInt((LocalPos.X + (LocalFlowField.GridSizeX * LocalFlowField.CellSize * 0.5f)) / LocalFlowField.CellSize);
	int32 Y = FMath::FloorToInt((LocalPos.Y + (LocalFlowField.GridSizeY * LocalFlowField.CellSize * 0.5f)) / LocalFlowField.CellSize);

	if (X >= 0 && X < LocalFlowField.GridSizeX && Y >= 0 && Y < LocalFlowField.GridSizeY)
	{
		return Y * LocalFlowField.GridSizeX + X;
	}
	return INDEX_NONE;
}

FVector ASquadManager::GetSquadCenterLocation() const
{
	FVector TotalLoc = FVector::ZeroVector;
	int32 ValidCount = 0;

	for (const ABaseCharacter* Member : SquadMembers)
	{
		if (Member)
		{
			TotalLoc += Member->GetActorLocation();
			ValidCount++;
		}
	}

	return ValidCount > 0 ? (TotalLoc / ValidCount) : GetActorLocation();
}

void ASquadManager::CalculateLocalFlowField(const FVector& TargetLocation, const TArray<FVector>& FormationOffsets)
{
	// FlowFieldSubsystem에서 지형 CostMap 정보만 참조하거나, Subsystem의 계산 연산을 LocalFlowField.UnitDirections로 받아옵니다.
	if (UFlowFieldSubsystem* FlowSys = GetWorld()->GetSubsystem<UFlowFieldSubsystem>())
	{
		// Subsystem 연산 결과를 LocalFlowField.UnitDirections 배열에 저장
		FlowSys->CalculateLocalSquadFlowField(TargetLocation, FormationOffsets, LocalFlowField.UnitDirections);
	}
}
