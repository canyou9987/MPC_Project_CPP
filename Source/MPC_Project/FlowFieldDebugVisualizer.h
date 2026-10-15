// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowFieldDebugVisualizer.generated.h"

UENUM(BlueprintType)
enum class EFlowFieldDebugMode : uint8
{
	CostField        UMETA(DisplayName = "1. Cost Field (Red/Green)"),
	IntegrationField UMETA(DisplayName = "2. Integration Field (Gradient)"),
	FlowField        UMETA(DisplayName = "3. Flow Field (Vector Arrows)"),
	Off              UMETA(DisplayName = "4. Off")
};

UCLASS()
class MPC_PROJECT_API AFlowFieldDebugVisualizer : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	AFlowFieldDebugVisualizer();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawGrid = true;

	// 에디터 디테일 패널에서 모드를 드롭다운으로 변경 가능!
	UPROPERTY(EditAnywhere, Category = "Debug")
	EFlowFieldDebugMode DebugMode = EFlowFieldDebugMode::IntegrationField;
};
