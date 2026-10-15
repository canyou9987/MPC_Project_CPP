// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractiveDoor.generated.h"

class UStaticMeshComponent;

UCLASS()
class MPC_PROJECT_API AInteractiveDoor : public AActor , public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// --- IInteractableInterface 오버라이딩 ---
	virtual EActionTargetType GetActionTargetType() const override { return InteractionInfo.InteractionType; }
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FInteractionInfo GetInteractionInfo() const override { return InteractionInfo; }
	virtual bool OnInteract(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;

	// 문 상태 세팅 함수
	void SetDoorState(bool bOpen);

protected:
	// 문 틀 및 문짝 프레임
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorFrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FInteractionInfo InteractionInfo;

	// 잠금 상태 (true일 경우 DC 체크 필수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsLocked = false;

	// 열림 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsOpen = false;

	// 문 회전 각도 (열릴 때 90도 회전)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenAngle = 90.0f;

	FRotator ClosedRotation;
	FRotator OpenedRotation;
};
