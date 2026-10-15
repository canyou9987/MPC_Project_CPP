// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractComponent.h"
#include "DiceManager.h"
#include "CombatStatInterface.h"
#include "GameFramework/Character.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	// 평시에는 틱을 꺼두고 상호작용 명령이 들어왔을 때만 활성화
	SetComponentTickEnabled(false);
	
}


// Called every frame
void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 대상이 파괴되었거나 유효하지 않으면 정리
	if (!TargetInteractableActor)
	{
		ClearInteractTarget();
		return;
	}

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(TargetInteractableActor);
	if (!Interactable || !Interactable->CanInteract(GetOwner()))
	{
		ClearInteractTarget();
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FInteractionInfo Info = Interactable->GetInteractionInfo();
	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), TargetInteractableActor->GetActorLocation());
	float RangeSq = FMath::Square(Info.InteractionRange);

	// 1. 사거리 이내에 도달한 경우
	if (DistSq <= RangeSq)
	{
		// 이동 멈춤
		if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
		{
			if (AController* Controller = OwnerChar->GetController())
			{
				Controller->StopMovement();
			}
		}

		ExecuteInteraction();
		ClearInteractTarget();
	}

	// 2. 아직 사거리 밖인 경우 -> 대상으로 이동 명령
	else if (!bIsMovingToInteract)
	{
		if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(OwnerChar->GetController(), TargetInteractableActor->GetActorLocation());
			bIsMovingToInteract = true;
		}
	}
}

bool UInteractComponent::RequestInteractCommand(AActor* InTargetActor)
{
	if (!InTargetActor) return false;

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(InTargetActor);
	if (!Interactable || !Interactable->CanInteract(GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Interact with %s"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"), *InTargetActor->GetName());
		return false;
	}

	TargetInteractableActor = InTargetActor;
	bIsMovingToInteract = false;

	// 추적 및 거리 검사를 위해 틱 활성화
	SetComponentTickEnabled(true);
	return true;
}

void UInteractComponent::ClearInteractTarget()
{
	TargetInteractableActor = nullptr;
	bIsMovingToInteract = false;

	SetComponentTickEnabled(false);
}

void UInteractComponent::ExecuteInteraction()
{
	if (!TargetInteractableActor) return;

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(TargetInteractableActor);
	if (!Interactable) return;

	AActor* Owner = GetOwner();
	FInteractionInfo Info = Interactable->GetInteractionInfo();

	// 1. 기술 굴림(Skill Check)이 필요한 상호작용 (예: 자물쇠 따기 / 함정 해제)
	if (Info.bRequiresSkillCheck)
	{
		int32 SkillBonus = 0;

		// ICombatStatInterface를 사용해 캐릭터의 민첩/손재주 수정치 수집
		if (ICombatStatInterface* StatOwner = Cast<ICombatStatInterface>(Owner))
		{
			SkillBonus = StatOwner->GetSavingThrowBonus(ESavingThrowType::Reflex); // 민첩 수정치 기반
		}

		FD20RollResult RollResult;
		bool bPassed = UDiceManager::CheckSavingThrow(SkillBonus, Info.DifficultyClass, RollResult);

		if (bPassed)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Skill Check PASSED vs DC %d (Roll: d20(%d) + Bonus(%d) = %d)"),
				*Owner->GetName(), Info.DifficultyClass, RollResult.RawRoll, RollResult.Bonus, RollResult.Total);

			// 성공 시 상호작용 발생
			Interactable->OnInteract(Owner);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Skill Check FAILED vs DC %d (Roll: d20(%d) + Bonus(%d) = %d)"),
				*Owner->GetName(), Info.DifficultyClass, RollResult.RawRoll, RollResult.Bonus, RollResult.Total);

			// 실패 처리 (필요시 실패 전용 이벤트 확장)
		}
	}
	else
	{
		// 2. 기술 굴림이 필요 없는 즉시 상호작용 (단순 루팅, 열린 문, NPC 대화 등)
		Interactable->OnInteract(Owner);
	}
}

