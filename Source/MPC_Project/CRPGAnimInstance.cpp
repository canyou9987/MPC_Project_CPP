// Fill out your copyright notice in the Description page of Project Settings.


#include "CRPGAnimInstance.h"
#include "AnimStateInterface.h"
#include "CombatComponent.h"
#include "AbilityComponent.h"
#include "GameFramework/Pawn.h"

void UCRPGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UCRPGAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner) return;

	// IAnimStateInterface 구현체 체크
	if (IAnimStateInterface* AnimProvider = Cast<IAnimStateInterface>(PawnOwner))
	{
		// 1. 속도 및 이동 상태
		GroundSpeed = AnimProvider->GetCurrentMovementSpeed();
		bIsMoving = GroundSpeed > 3.0f;

		// 2. 애니메이션 상태
		CurrentAnimState = AnimProvider->GetCurrentAnimState();
	}
	else
	{
		// 인터페이스 미구현 기본 Pawn 대응 (Velocity 기반)
		GroundSpeed = PawnOwner->GetVelocity().Size2D();
		bIsMoving = GroundSpeed > 3.0f;
	}
}

void UCRPGAnimInstance::AnimNotify_OnHitImpact()
{
	UE_LOG(LogTemp, Warning, TEXT("================> [AnimBP] OnHitImpact Notify Triggered! <================"));

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner) return;

	if (UCombatComponent* CombatComp = PawnOwner->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->OnHitImpactNotify();
	}
}

void UCRPGAnimInstance::AnimNotify_OnSpawnProjectile()
{
	UE_LOG(LogTemp, Warning, TEXT("================> [AnimBP] OnSpawnProjectile Notify Triggered! <================"));

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner) return;

	if (UCombatComponent* CombatComp = PawnOwner->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->OnSpawnProjectileNotify();
	}
}

void UCRPGAnimInstance::AnimNotify_OnSpellCast()
{
	UE_LOG(LogTemp, Warning, TEXT("================> [AnimBP] OnSpellCast Notify Triggered! <================"));

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner) return;

	// 소유 폰에서 AbilityComponent를 찾아 대기 중인 스킬 효과 집행!
	if (UAbilityComponent* AbilityComp = PawnOwner->FindComponentByClass<UAbilityComponent>())
	{
		AbilityComp->ExecutePendingSpellEffect();
	}
}
