// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableInterface.h"
#include "InteractComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPC_PROJECT_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상호작용 명령 시작 (CameraPawn 또는 PlayerController에서 호출)
	bool RequestInteractCommand(AActor* InTargetActor);

	// 현재 상호작용 목표 취소 및 초기화
	void ClearInteractTarget();

	// 상호작용 진행 중 여부
	bool HasInteractTarget() const { return TargetInteractableActor != nullptr; }

protected:
	// 사거리에 도달했을 때 기술 굴림 및 상호작용 집행
	void ExecuteInteraction();

protected:
	// 상호작용 대상 액터
	UPROPERTY()
	TObjectPtr<AActor> TargetInteractableActor;

	// 이동 중 플래그
	bool bIsMovingToInteract = false;
		
};
