// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionTargetType.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

// 상호작용 요구 조건 및 기술 굴림 정보
USTRUCT(BlueprintType)
struct FInteractionInfo
{
	GENERATED_BODY()

	// 기존 EActionTargetType (Interactable, Loot, Friendly 등) 재활용!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EActionTargetType InteractionType = EActionTargetType::Interactable;

	// 기술 굴림 필요 여부 (예: 자물쇠 따기, 함정 해제)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bRequiresSkillCheck = false;

	// 난이도 DC (D20 + 보너스 >= DC)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	int32 DifficultyClass = 10;

	// 상호작용 사거리 (기본 150cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 150.0f;

	// 이미 열렸거나 해제되어 더 이상 상호작용 불가능한지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsAlreadyInteracted = false;
};

class MPC_PROJECT_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 상호작용 대상의 타입(Enemy, Loot, Friendly 등)을 반환하는 순수 가상 함수
	virtual EActionTargetType GetActionTargetType() const = 0;

	// 1. 현재 상호작용 가능한 상태인가?
	virtual bool CanInteract(AActor* Interactor) const = 0;

	// 2. 상호작용 정보(DC, 사거리, 타입 등) 가져오기
	virtual FInteractionInfo GetInteractionInfo() const = 0;

	// 3. 실제 상호작용 실행 (주사위 판정 성공 후 또는 즉시 실행)
	virtual bool OnInteract(AActor* Interactor) = 0;

	// UI 프롬프트에 출력할 안내 문구 반환 함수 추가
	virtual FText GetInteractionPrompt() const = 0;
};
