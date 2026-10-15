// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "GameFramework/Actor.h"
#include "SquadManager.generated.h"

class USquadSpawnerComponent;
class ABaseCharacter;

// 진형 종류 (방진 / 횡대 / 종대)
UENUM(BlueprintType)
enum class ESquadFormationType : uint8
{
	Grid,       // 방진 (Square / Rectangle Grid)
	Line,       // 횡대 (Wide Line - 한 줄로 길게)
	Column      // 종대 (Column - 1열 종대)
};

// 스쿼드 자체 Local FlowField 데이터를 담을 구조체
USTRUCT(BlueprintType)
struct FSquadFlowFieldData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 GridSizeX = 50;

	UPROPERTY()
	int32 GridSizeY = 50;

	UPROPERTY()
	float CellSize = 100.0f;

	UPROPERTY()
	FVector GridOrigin = FVector::ZeroVector;

	// 이 스쿼드 노드별 이동 방향 벡터 배열
	TArray<FVector> UnitDirections;

	// 노드별 Integration Cost (필요 시)
	TArray<uint16> IntegrationCosts;

	void Initialize(int32 InSizeX, int32 InSizeY, float InCellSize, FVector InOrigin)
	{
		GridSizeX = InSizeX;
		GridSizeY = InSizeY;
		CellSize = InCellSize;
		GridOrigin = InOrigin;

		int32 TotalNodes = GridSizeX * GridSizeY;
		UnitDirections.Init(FVector::ZeroVector, TotalNodes);
		IntegrationCosts.Init(65535, TotalNodes);
	}
};


UCLASS()
class MPC_PROJECT_API ASquadManager : public AActor, public ISelectableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASquadManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	// 부대,부대원,진형 에 대한 변수들
	// =========================================================================
	// 1. 부대 식별 및 부대원 목록
	// =========================================================================
	// 부대 고유 번호 (A부대: 1, B부대: 2 등 - 부대별 FlowField 레이어 구분에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 SquadID = 1;

	// 이 부대에 속한 캐릭터 목록
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Squad")
	TArray<ABaseCharacter*> SquadMembers;

	// 스쿼드 소유의 Local FlowField 데이터
	FSquadFlowFieldData LocalFlowField;

	// =========================================================================
	// 2. 진형(Formation) 설정
	// =========================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Formation")
	ESquadFormationType FormationType = ESquadFormationType::Grid;

	// 유닛 간 간격 (cm 단위, 기본 150cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Formation")
	float UnitSpacing = 150.0f;

	// 한 줄당 최대 유닛 수 (Grid 방진 기준, 예: 5명씩 한 줄)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad|Formation")
	int32 MaxUnitsPerRow = 5;

protected:
	// 스포너 컴포넌트 서브오브젝트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad|Components")
	USquadSpawnerComponent* SpawnerComponent;

public:
	// =========================================================================
	// 3. ISelectableInterface 오버라이딩 (부대 단위 선택/이동 위임)
	// =========================================================================
	virtual void SetSelected(bool bSelected) override;
	virtual bool IsSelected() const override { return bIsSquadSelected; }
	virtual void StartFlowFieldMovement() override;
	virtual void StopFlowFieldMovement() override;

	virtual EActionTargetType GetActionTargetType() const override { return EActionTargetType::Friendly; }
	virtual void CommandTargetAction(AActor* InTargetActor, EActionTargetType TargetType) override;
	virtual void CommandTargetAction(EActionTargetType TargetType, const FVector& TargetLocation) override;
	virtual void ToggleHoldPosition() override;
	virtual void ToggleAutoCombat() override;

	virtual ISelectableInterface* GetSelectionOwner() override { return this; }

	// =========================================================================
	// 4. 부대 관리 및 진형 좌표 연산 함수
	// =========================================================================
	// 부대원 등록 / 해제
	void RegisterMember(ABaseCharacter* NewMember);
	void UnregisterMember(ABaseCharacter* MemberToUnregister);

	// 부대 이동 명령 하달 (우클릭 지점)
	//void CommandSquadMoveTo(const FVector& TargetLocation);

	// 120마리 부대원 개별 목표 오프셋 좌표 연산 함수 (핵심 수학 공식!)
	TArray<FVector> CalculateFormationOffsets(const FVector& TargetLocation, const FVector& MovementDirection);

	// [추가] 부원 캐릭터가 위치(WorldLocation)를 주면 자신의 Local FlowField에서 이동 방향을 반환하는 함수
	FVector GetMoveDirectionForUnit(const FVector& UnitLocation) const;

	// [추가] World 좌표를 그리드 노드 인덱스로 변환하는 헬퍼 함수
	int32 WorldToGridIndex(const FVector& WorldLocation) const;


public:

	// 이동 진행 여부 플래그
	bool bHasActiveMoveCommand = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	FVector CurrentMainTargetLocation;

	// 부대원별 최종 목표 위치 (절대 좌표 저장용 맵)
	UPROPERTY()
	TMap<ABaseCharacter*, FVector> MemberTargetLocations;

private:
	bool bIsSquadSelected = false;

	// 현재 부대원들의 중심 위치(Pivot) 계산
	FVector GetSquadCenterLocation() const;

protected:
	// 스쿼드 내부에서 Local FlowField 연산 수행
	void CalculateLocalFlowField(const FVector& TargetLocation, const TArray<FVector>& FormationOffsets);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



};
