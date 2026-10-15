// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractiveChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class MPC_PROJECT_API AInteractiveChest : public AActor , public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveChest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// --- IInteractableInterface ±¸Çö ---
	virtual EActionTargetType GetActionTargetType() const override { return InteractionInfo.InteractionType; }
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FInteractionInfo GetInteractionInfo() const override { return InteractionInfo; }
	virtual bool OnInteract(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FInteractionInfo InteractionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	bool bIsOpened = false;

};
