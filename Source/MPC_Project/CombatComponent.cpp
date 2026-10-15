// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ActionEconomyComponent.h"
#include "CombatStatInterface.h"
#include "DamageableInterface.h"
#include "DiceManager.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "TurnManagerSubsystem.h"
#include "MovableInterface.h"
#include "ProjectileActor.h"
#include "AnimStateInterface.h"
#include "AnimationComponent.h"
#include "BuffComponent.h"
#include "EquipmentComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bIsMovingToAttack = false;
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	if (OwnerActor)
	{
		ActionEconomyComp = OwnerActor->FindComponentByClass<UActionEconomyComponent>();

		// =========================================================
		// [추가] EquipmentComponent 델리게이트 구독 (구독신청)
		// =========================================================
		if (UEquipmentComponent* EquipComp = OwnerActor->FindComponentByClass<UEquipmentComponent>())
		{
			EquipComp->OnWeaponDataChanged.AddUObject(this, &UCombatComponent::HandleWeaponDataChanged);

			// 시작 시점 이미 장착된 무기 데이터가 있다면 바로 동기화
			HandleWeaponDataChanged(EquipComp->GetCurrentWeaponData());
		}

	}

	PrimaryComponentTick.SetTickFunctionEnable(false);
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// OwnerActor가 없으면 진행 불가
	if (!OwnerActor) return;

	// 타깃이 없으면 틱을 끄고 리턴
	if (!CombatTarget)
	{
		SetComponentTickEnabled(false);
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("[%s] CombatComponent Tick Executing! Target: %s"),
	//	*OwnerActor->GetName(), *CombatTarget->GetName());

	// 1. 타깃 사망 여부 확인
	if (IDamageableInterface* DamageableTarget = Cast<IDamageableInterface>(CombatTarget))
	{
		if (DamageableTarget->GetCurrentHP() <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Target is dead! Clearing target."), *OwnerActor->GetName());
			ClearCombatTarget();
			return;
		}
	}

	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	const bool bInCombat = TurnSys && TurnSys->IsCombatActive();

	// 2. 사거리 내 진입 여부 판정
	if (IsTargetInWeaponRange())
	{
		if (bIsMovingToAttack)
		{
			bIsMovingToAttack = false;
			RequestOwnerStop();
		}

		if (CanAttack())
		{
			ExecuteAttack(CombatTarget, bInCombat);

			// 턴 모드에서는 공격 후 1회 수행 완료 -> 명령 종료
			if (bInCombat)
			{
				ClearCombatTarget();
			}
		}
	}
	else
	{
		// 3. 사거리 밖 -> 적을 향해 이동 요청
		bool bCanMoveToTarget = true;

		// 턴제 전투 중일 때 이동 자원 검사
		if (bInCombat && ActionEconomyComp)
		{
			if (ActionEconomyComp->GetRemainingMoveDistance() <= 10.0f)
			{
				bCanMoveToTarget = false;
			}
		}

		if (bCanMoveToTarget)
		{
			bIsMovingToAttack = true;
			RequestOwnerMoveTo(CombatTarget->GetActorLocation());
		}
		else
		{
			bIsMovingToAttack = false;
			RequestOwnerStop();

			if (bInCombat)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot reach target: Remaining move distance depleted!"), *OwnerActor->GetName());
				ClearCombatTarget();
			}
		}
	}
}

bool UCombatComponent::RequestAttackCommand(AActor* InTargetActor, bool bInCombat, bool bIsMyTurn)
{
	if (!OwnerActor || !InTargetActor) return false;

	if (bInCombat && !bIsMyTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Not my turn! Attack Command Ignored."), *OwnerActor->GetName());
		return false;
	}

	if (bInCombat && ActionEconomyComp && !ActionEconomyComp->HasStandardAction())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Command Rejected: Standard Action already used this turn!"), *OwnerActor->GetName());
		return false;
	}

	CombatTarget = InTargetActor;
	bIsMovingToAttack = false;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Received Attack Command on Target: %s"),
		*OwnerActor->GetName(), *InTargetActor->GetName());

	// 공격 명령이 들어왔으므로 스스로 Tick을 켬!
	SetComponentTickEnabled(true);
	return true;
}

void UCombatComponent::StopAttackCommand()
{
	ClearCombatTarget();
	RequestOwnerStop();
}

bool UCombatComponent::CanAttack() const
{
	if (!OwnerActor) return false;
	/*
	// 1. 군중 제어(CC) 상태 검사
	if (UBuffComponent* BuffComp = OwnerActor->FindComponentByClass<UBuffComponent>())
	{
		if (BuffComp->HasState(ECharacterState::Stunned) || BuffComp->HasState(ECharacterState::Paralyzed))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Attack: Character is Stunned/Paralyzed!"), *OwnerActor->GetName());
			return false;
		}
	}
	*/
	// 2. Standard Action 존재 여부로 공격 가능 검사 (전투/탐험 통합)
	if (ActionEconomyComp)
	{
		return ActionEconomyComp->HasStandardAction();
	}

	return false;
}

void UCombatComponent::ExecuteAttack(AActor* Target, bool bInCombat)
{
	if (!OwnerActor || !Target) return;

	// Standard Action 소모 (전투/탐험 통합)
	if (!ActionEconomyComp || !ActionEconomyComp->ConsumeStandardAction())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Attack Failed: Standard Action already used!"), *OwnerActor->GetName());
		ClearCombatTarget();
		return;
	}

	FVector Direction = Target->GetActorLocation() - OwnerActor->GetActorLocation();
	Direction.Z = 0.0f;
	OwnerActor->SetActorRotation(Direction.Rotation());

	int32 AttackBonus = 0;
	int32 DamageBonus = 0;

	if (ICombatStatInterface* StatOwner = Cast<ICombatStatInterface>(OwnerActor))
	{
		EWeaponRangeType RangeType = CurrentWeaponData.WeaponProperties.bIsRanged ? EWeaponRangeType::Ranged : EWeaponRangeType::Melee;
		AttackBonus = StatOwner->GetAttackBonus(RangeType);
		DamageBonus = StatOwner->GetDamageBonus();
	}

	int32 TargetAC = 10;
	if (IDamageableInterface* DamageableTarget = Cast<IDamageableInterface>(Target))
	{
		TargetAC = DamageableTarget->GetArmorClass();
	}

	FD20RollResult AttackRoll = UDiceManager::RollD20(AttackBonus);
	bool bIsHit = UDiceManager::CheckHit(AttackRoll, TargetAC);

	int32 DiceCount = CurrentWeaponData.DamageDiceCount > 0 ? CurrentWeaponData.DamageDiceCount : 1;
	int32 DieSides = CurrentWeaponData.DamageDiceSides > 0 ? CurrentWeaponData.DamageDiceSides : 4;
	int32 Damage = bIsHit ? UDiceManager::RollDamage(DiceCount, DieSides, DamageBonus) : 0;

	PendingTarget = Target;
	bPendingIsHit = bIsHit;
	PendingDamage = Damage;

	if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(OwnerActor))
	{
		AnimOwner->SetCurrentAnimState(ECharacterAnimState::Attacking);
	}

	UAnimationComponent* AnimComp = OwnerActor->FindComponentByClass<UAnimationComponent>();
	float MontageLength = 0.0f;
	if (AnimComp)
	{
		MontageLength = AnimComp->PlayWeaponAttackMontage(AttackMontage);
	}

	if (MontageLength <= 0.0f)
	{
		if (CurrentWeaponData.WeaponProperties.bIsRanged)
			OnSpawnProjectileNotify();
		else
			OnHitImpactNotify();
	}

	if (bInCombat)
	{
		ClearCombatTarget();
	}
}
void UCombatComponent::ClearCombatTarget()
{
	CombatTarget = nullptr;
	bIsMovingToAttack = false;

	SetComponentTickEnabled(false);
}

float UCombatComponent::GetDistanceToTarget() const
{
	if (!OwnerActor || !CombatTarget) return 0.0f;
	return FVector::Distance(OwnerActor->GetActorLocation(), CombatTarget->GetActorLocation());
}

bool UCombatComponent::IsTargetInWeaponRange() const
{
	if (!OwnerActor || !CombatTarget) return false;
	float AttackRange = CurrentWeaponData.AttackRange > 0.0f ? CurrentWeaponData.AttackRange : 150.0f;
	return GetDistanceToTarget() <= AttackRange;
}

void UCombatComponent::OnHitImpactNotify()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] OnHitImpactNotify Called inside CombatComponent!"), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));

	if (!PendingTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OnHitImpactNotify Failed: PendingTarget is NULL!"), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));
		return;
	}

	if (!CurrentWeaponData.WeaponProperties.bIsRanged)
	{
		if (bPendingIsHit)
		{
			if (IDamageableInterface* DamageableTarget = Cast<IDamageableInterface>(PendingTarget))
			{
				DamageableTarget->ApplyDamageAmount(PendingDamage, OwnerActor);
				UE_LOG(LogTemp, Warning, TEXT("[%s] Applied %d Damage to %s!"), *OwnerActor->GetName(), PendingDamage, *PendingTarget->GetName());
			}
		}
	}

	if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(OwnerActor))
	{
		AnimOwner->SetCurrentAnimState(ECharacterAnimState::IdleRun);
	}

	PendingTarget = nullptr;
}

void UCombatComponent::OnSpawnProjectileNotify()
{
	if (!PendingTarget) return;

	if (CurrentWeaponData.WeaponProperties.bIsRanged && CurrentWeaponData.ProjectileClass)
	{
		FVector SpawnLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 80.0f + FVector(0.0f, 0.0f, 50.0f);
		FRotator SpawnRotation = (PendingTarget->GetActorLocation() - SpawnLocation).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor;
		SpawnParams.Instigator = Cast<APawn>(OwnerActor);

		AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(
			CurrentWeaponData.ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			Projectile->InitProjectile(OwnerActor, PendingTarget, bPendingIsHit, PendingDamage);
		}
	}

	// 상태 복구
	if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(OwnerActor))
	{
		AnimOwner->SetCurrentAnimState(ECharacterAnimState::IdleRun);
	}

	PendingTarget = nullptr;
}

void UCombatComponent::RequestOwnerMoveTo(const FVector& TargetLocation)
{
	// 소유자 액터가 IMovableInterface를 구현하고 있다면 단순 호출!
	if (IMovableInterface* MovableOwner = Cast<IMovableInterface>(OwnerActor))
	{
		MovableOwner->MoveToLocation(TargetLocation);
	}
}

void UCombatComponent::RequestOwnerStop()
{
	if (IMovableInterface* MovableOwner = Cast<IMovableInterface>(OwnerActor))
	{
		MovableOwner->StopMovement();
	}
}

void UCombatComponent::HandleWeaponDataChanged(const FWeaponDataRow& NewWeaponData)
{
	CurrentWeaponData = NewWeaponData;

	UE_LOG(LogTemp, Warning, TEXT("[%s][CombatComponent] Weapon Data Synced via Delegate: %s (Damage: %dd%d, Range: %.1f)"),
		OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
		*CurrentWeaponData.WeaponName.ToString(),
		CurrentWeaponData.DamageDiceCount,
		CurrentWeaponData.DamageDiceSides,
		CurrentWeaponData.AttackRange);
}