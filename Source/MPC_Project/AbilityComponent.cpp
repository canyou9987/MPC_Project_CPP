// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityComponent.h"
#include "ActionEconomyComponent.h"
#include "BuffComponent.h"
#include "BuffableInterface.h"
#include "DamageableInterface.h"
#include "DiceManager.h"
#include "TurnManagerSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CombatStatInterface.h"
#include "AnimStateInterface.h"
#include "AnimationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

// Sets default values for this component's properties
UAbilityComponent::UAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAbilityComponent::BeginPlay()
{
    Super::BeginPlay();

    //  평시에는 Tick을 끈다
    SetComponentTickEnabled(false);

}


// Called every frame
void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Tick이 켜진 상태(조준 중)일 때만 마우스 커서를 바라봄
    RotateTowardsMouseCursor(DeltaTime);

}

void UAbilityComponent::SetPendingSkill(const FDataTableRowHandle& SkillRow)
{
    // 같은 스킬을 재호출했을 때 Cancel을 부르지 않고 무시(return)하도록 수정!
    if (HasPendingSkill() && PendingSkillRow.RowName == SkillRow.RowName)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 이미 대기 중인 스킬입니다. 중복 호출 무시."), *SkillRow.RowName.ToString());
        return;
    }

    if (SkillRow.IsNull())
    {
        ClearPendingSkill();
        return;
    }

    // 다른 스킬이 이미 대기 중이었다면 가볍게 비워주고 새로 세팅
    if (HasPendingSkill())
    {
        ClearPendingSkill();
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FSkillDataRow* Data = SkillRow.GetRow<FSkillDataRow>(TEXT("PendingSkill Context"));
    if (!Data) return;

    // 행동력 자원 사전 검사
    UActionEconomyComponent* ActionComp = Owner->FindComponentByClass<UActionEconomyComponent>();
    if (!ActionComp || !CheckActionEconomyResource(ActionComp, Data->ActionCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Enter Spell Stance: Insufficient Action Economy Resource!"),
            *Owner->GetName());
        return;
    }

    PendingSkillRow = SkillRow;

    if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(Owner))
    {
        AnimOwner->SetCurrentAnimState(ECharacterAnimState::CastingSpell);
    }

    if (Data->CastLoopMontage)
    {
        if (UAnimationComponent* AnimComp = Owner->FindComponentByClass<UAnimationComponent>())
        {
            AnimComp->PlayCastMontage(Data->CastLoopMontage);
        }
    }

    SetComponentTickEnabled(true);

    UE_LOG(LogTemp, Warning, TEXT("[%s] Spell Stance Active: %s"),
        *Owner->GetName(), *PendingSkillRow.RowName.ToString());
}

void UAbilityComponent::SetPendingSkillBySlot(int32 SlotIndex)
{
    if (LearnedSkills.IsValidIndex(SlotIndex))
    {
        SetPendingSkill(LearnedSkills[SlotIndex]);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Invalid Skill Slot Index: %d"),
            GetOwner() ? *GetOwner()->GetName() : TEXT("None"), SlotIndex);
    }
}

void UAbilityComponent::ClearPendingSkill()
{
    if (!PendingSkillRow.IsNull())
    {
        AActor* Owner = GetOwner();

        // 1. IdleRun으로 복귀
        if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(Owner))
        {
            AnimOwner->SetCurrentAnimState(ECharacterAnimState::IdleRun);
        }

        // 2. 캐스팅 Loop 몽타주 중단
        if (UAnimationComponent* AnimComp = Owner->FindComponentByClass<UAnimationComponent>())
        {
            AnimComp->StopCurrentMontage(0.2f);
        }

        UE_LOG(LogTemp, Log, TEXT("[%s] Spell Stance Cancelled & Montage Stopped."),
            Owner ? *Owner->GetName() : TEXT("None"));
    }

    PendingSkillRow = FDataTableRowHandle();

    // 조준 취소 시 Component Tick 끄기
    SetComponentTickEnabled(false);
}

bool UAbilityComponent::ExecutePendingSkill(AActor* TargetActor, const FVector& TargetLocation)
{
    if (PendingSkillRow.IsNull()) return false;

    // 1. 미습득 스킬 자동 학습
    if (!HasLearnedSkill(PendingSkillRow))
    {
        LearnSkill(PendingSkillRow);
    }

    // 2. 스킬 시전 시도
    bool bSuccess = CastSkill(PendingSkillRow, TargetActor, TargetLocation);

    // 3. CastSkill이 '실패'했을 때만 Pending을 지워줍니다.
    if (!bSuccess)
    {
        ClearPendingSkill();
        UE_LOG(LogTemp, Warning, TEXT("[BP_BaseCharacter] Spell Stance Cancelled by %s"), ANSI_TO_TCHAR(__FUNCTION__));
    }
    else
    {
        // ExecutePendingSkill이 성공했으므로 PendingSkillRow만 비워줍니다.
        PendingSkillRow = FDataTableRowHandle();
    }

    return bSuccess;
}

void UAbilityComponent::ExecutePendingSpellEffect()
{
    if (!PendingCastData.bIsValid || !PendingCastData.SkillData) return;

    AActor* Owner = GetOwner();
    const FSkillDataRow* Data = PendingCastData.SkillData;

    // VFX 스폰 위치 결정 (타깃 액터가 있으면 액터 위치, 없으면 지정 지점 위치)
    FVector ImpactLocation = PendingCastData.TargetActor ? PendingCastData.TargetActor->GetActorLocation() : PendingCastData.TargetLocation;

    // 1. 스킬 명중/폭발 시각 이펙트 스폰!
    SpawnSkillImpactVFX(Data, ImpactLocation);

    // -----------------------------------------------------------------
    // 1. 투사체(Projectile) 스폰 처리
    // -----------------------------------------------------------------
    if (Data->ProjectileClass)
    {
        FVector SpawnLocation = Owner ? Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f + FVector(0, 0, 50) : FVector::ZeroVector;
        FRotator SpawnRotation = Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator;

        if (PendingCastData.TargetActor)
        {
            SpawnRotation = (PendingCastData.TargetActor->GetActorLocation() - SpawnLocation).Rotation();
        }
        else if (!PendingCastData.TargetLocation.IsZero())
        {
            SpawnRotation = (PendingCastData.TargetLocation - SpawnLocation).Rotation();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Owner;
        SpawnParams.Instigator = Cast<APawn>(Owner);

        // 투사체 액터 월드 스폰!
        AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(Data->ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

        // (투사체가 이니셜라이즈 인터페이스나 타깃 세팅 함수를 가지고 있다면 여기서 TargetActor/Location 전달)
    }

    // -----------------------------------------------------------------
    // 2. 백엔드 데미지/내성 굴림 집행 (기존 로직 그대로 유지)
    // -----------------------------------------------------------------
    switch (Data->TargetType)
    {
    case ESpellTargetType::SingleTarget:
        ApplySkillEffectToTarget(Data, PendingCastData.TargetActor);
        break;

    case ESpellTargetType::PointLocation:
        ApplySkillEffectToArea(Data, PendingCastData.TargetLocation);
        break;

    case ESpellTargetType::AreaOfEffect:
        ApplySkillEffectToArea(Data, Owner ? Owner->GetActorLocation() : FVector::ZeroVector);
        break;

    case ESpellTargetType::Self:
        ApplySkillEffectToTarget(Data, Owner);
        break;
    }

    // 시전 완료 후 애니메이션 상태를 다시 IdleRun으로 복구
    if (IAnimStateInterface* AnimOwner = Cast<IAnimStateInterface>(Owner))
    {
        AnimOwner->SetCurrentAnimState(ECharacterAnimState::IdleRun);
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s] Executed Pending Spell Effect: %s"),
        Owner ? *Owner->GetName() : TEXT("None"), *Data->SkillName.ToString());

    PendingCastData.Clear();
}

bool UAbilityComponent::CanCastSkill(const FDataTableRowHandle& SkillRow, AActor* TargetActor, FVector TargetLocation) const
{
    if (SkillRow.IsNull()) return false;
    const FSkillDataRow* Data = SkillRow.GetRow<FSkillDataRow>(TEXT("CanCastSkill Context"));
    if (!Data) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    // 1. 학습 여부 검사
    if (!HasLearnedSkill(SkillRow))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Cast %s: Skill Not Learned!"),
            *Owner->GetName(), *Data->SkillName.ToString());
        return false;
    }

    // 2. 행동력 자원 검사 (전투/탐험 모드 구분 없이 통합 검사)
    UActionEconomyComponent* ActionComp = Owner->FindComponentByClass<UActionEconomyComponent>();
    if (!ActionComp || !CheckActionEconomyResource(ActionComp, Data->ActionCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Cast %s: Insufficient Action Economy Resource!"),
            *Owner->GetName(), *Data->SkillName.ToString());
        return false;
    }

    // 3. 사거리 검사
    const FVector OwnerLoc = Owner->GetActorLocation();
    const float CastRangeSq = FMath::Square(Data->CastRange);

    if (Data->TargetType == ESpellTargetType::SingleTarget)
    {
        if (!TargetActor) return false;
        if (FVector::DistSquared(OwnerLoc, TargetActor->GetActorLocation()) > CastRangeSq)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Cast %s: Target out of range!"),
                *Owner->GetName(), *Data->SkillName.ToString());
            return false;
        }
    }
    else if (Data->TargetType == ESpellTargetType::PointLocation)
    {
        if (FVector::DistSquared(OwnerLoc, TargetLocation) > CastRangeSq)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot Cast %s: Target Location out of range!"),
                *Owner->GetName(), *Data->SkillName.ToString());
            return false;
        }
    }

    return true;
}

bool UAbilityComponent::CastSkill(const FDataTableRowHandle& SkillRow, AActor* TargetActor, FVector TargetLocation)
{
    if (!CanCastSkill(SkillRow, TargetActor, TargetLocation))
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Cannot Cast Skill!"), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
        return false;
    }

    const FSkillDataRow* Data = SkillRow.GetRow<FSkillDataRow>(TEXT("CastSkill Context"));
    if (!Data) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    // 1. 타깃 방향 회전 고정 및 틱 Off
    FVector FinalTargetPos = TargetActor ? TargetActor->GetActorLocation() : TargetLocation;
    FVector Direction = FinalTargetPos - Owner->GetActorLocation();
    Direction.Z = 0.0f;

    if (!Direction.IsNearlyZero())
    {
        Owner->SetActorRotation(Direction.Rotation());
    }

    SetComponentTickEnabled(false);

    // 2. 행동력 실제 소모 (전투/탐험 모드 구분 없이 차감)
    UActionEconomyComponent* ActionComp = Owner->FindComponentByClass<UActionEconomyComponent>();
    if (!ActionComp || !ConsumeActionEconomyResource(ActionComp, Data->ActionCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to consume Action Economy resource!"), *Owner->GetName());
        return false;
    }

    // 3. PendingCastData 설정
    PendingCastData.SkillData = Data;
    PendingCastData.TargetActor = TargetActor;
    PendingCastData.TargetLocation = TargetLocation;
    PendingCastData.bIsValid = true;

    // 4. 캐스팅 몽타주 재생
    bool bMontageStarted = false;
    UAnimMontage* MontageToPlay = Data->CastReleaseMontage ? Data->CastReleaseMontage.Get() : Data->CastMontage.Get();

    if (UAnimationComponent* AnimComp = Owner->FindComponentByClass<UAnimationComponent>())
    {
        if (MontageToPlay)
        {
            float Duration = AnimComp->PlayCastMontage(MontageToPlay);
            bMontageStarted = (Duration > 0.0f);
        }
    }

    if (!bMontageStarted)
    {
        ExecutePendingSpellEffect();
    }

    return true;
}

void UAbilityComponent::LearnSkill(FDataTableRowHandle NewSkillRow)
{
    if (NewSkillRow.IsNull() || HasLearnedSkill(NewSkillRow)) return;
    LearnedSkills.Add(NewSkillRow);
}

bool UAbilityComponent::HasLearnedSkill(const FDataTableRowHandle& SkillRow) const
{
    for (const FDataTableRowHandle& Row : LearnedSkills)
    {
        if (Row.RowName == SkillRow.RowName && Row.DataTable == SkillRow.DataTable)
            return true;
    }
    return false;
}

bool UAbilityComponent::GetSkillDataBySlot(int32 SlotIndex, FSkillDataRow& OutSkillData) const
{
    // 1. 슬롯 인덱스 유효성 검사
    if (!LearnedSkills.IsValidIndex(SlotIndex))
    {
        return false;
    }

    const FDataTableRowHandle& RowHandle = LearnedSkills[SlotIndex];

    // 2. 데이터 테이블 핸들이 유효한지 확인
    if (RowHandle.IsNull())
    {
        return false;
    }

    // 3. RowHandle에서 실제 FSkillDataRow 데이터 구조체 가져오기
    FSkillDataRow* FoundRow = RowHandle.GetRow<FSkillDataRow>(TEXT("GetSkillDataBySlotContext"));
    if (FoundRow)
    {
        OutSkillData = *FoundRow;
        return true;
    }

    return false;
}

bool UAbilityComponent::CheckActionEconomyResource(UActionEconomyComponent* ActionComp, EActionCostType CostType) const
{
    if (!ActionComp) return false;

    switch (CostType)
    {
    case EActionCostType::StandardAction: return ActionComp->HasStandardAction();
    case EActionCostType::MoveAction:     return ActionComp->HasMoveAction();
    case EActionCostType::SwiftAction:    return ActionComp->HasSwiftAction();
    case EActionCostType::FreeAction:     return true;
    }
    return false;
}

bool UAbilityComponent::ConsumeActionEconomyResource(UActionEconomyComponent* ActionComp, EActionCostType CostType)
{
    if (!ActionComp) return false;

    switch (CostType)
    {
    case EActionCostType::StandardAction: return ActionComp->ConsumeStandardAction();
    case EActionCostType::MoveAction:     return ActionComp->ConsumeMoveAction();
    case EActionCostType::SwiftAction:    return ActionComp->ConsumeSwiftAction();
    case EActionCostType::FreeAction:     return true;
    }
    return false;
}

void UAbilityComponent::RotateTowardsMouseCursor(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    FHitResult HitResult;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        FVector TargetLoc = HitResult.Location;
        FVector OwnerLoc = Owner->GetActorLocation();

        // Z축 높이는 무시하고 평면 회전
        FVector Direction = TargetLoc - OwnerLoc;
        Direction.Z = 0.0f;

        if (!Direction.IsNearlyZero())
        {
            FRotator TargetRotation = Direction.Rotation();
            FRotator CurrentRotation = Owner->GetActorRotation();

            // RInterpTo로 부드럽게 마우스 커서 쪽을 바라보게 처리
            FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SpellStanceRotateSpeed);
            Owner->SetActorRotation(NewRotation);
        }
    }
}

void UAbilityComponent::ApplySkillEffectToTarget(const FSkillDataRow* SkillData, AActor* TargetActor)
{
    if (!SkillData || !TargetActor) return;

    AActor* Owner = GetOwner();

    // 1. 버프/디버프 부여 (BuffComponent 책임 이관)
    if (!SkillData->AppliedBuffRow.IsNull())
    {
        if (IBuffableInterface* Buffable = Cast<IBuffableInterface>(TargetActor))
        {
            Buffable->ApplyBuffToSelf(SkillData->AppliedBuffRow);
        }
        else if (UBuffComponent* TargetBuffComp = TargetActor->FindComponentByClass<UBuffComponent>())
        {
            TargetBuffComp->ApplyBuff(SkillData->AppliedBuffRow);
        }
    }

    // 2. 데미지 / 회복 연산 처리
    if (SkillData->BaseDamageDiceCount > 0)
    {
        // 기본 주사위 굴림 (예: 1d8, 2d6 등)
        int32 RawValue = UDiceManager::RollDamage(SkillData->BaseDamageDiceCount, SkillData->BaseDamageDiceSides, 0);

        // A. 회복(Heal) 스킬 집행
        if (SkillData->ExecutionType == ESkillExecutionType::Heal)
        {
            if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(TargetActor))
            {
                // 명중/내성 굴림 없이 주사위 수치만큼 바로 인터페이스 회복 수용
                Damageable->ApplyHealingAmount(RawValue, Owner);
            }
            return; // 회복 처리 후 직행 종료
        }

        // B. 공격 / 내성 굴림 데미지 집행
        int32 FinalDamage = RawValue;

        // B-1. 내성 굴림 판정 (Saving Throw)
        if (SkillData->ExecutionType == ESkillExecutionType::SavingThrow)
        {
            int32 TargetSaveBonus = 0;

            if (ICombatStatInterface* StatTarget = Cast<ICombatStatInterface>(TargetActor))
            {
                TargetSaveBonus = StatTarget->GetSavingThrowBonus(SkillData->SaveType);
            }

            FD20RollResult SaveRollResult;
            bool bSavePassed = UDiceManager::CheckSavingThrow(TargetSaveBonus, SkillData->DifficultyClass, SaveRollResult);

            if (bSavePassed)
            {
                // D&D 3.5e 기본 규칙: 내성 성공 시 데미지 절반 (Half Damage)
                FinalDamage = FMath::Max(1, RawValue / 2);
                UE_LOG(LogTemp, Warning, TEXT("[%s] Saving Throw PASSED vs DC %d (d20:%d + Bonus:%d = %d) -> Reduced Damage: %d"),
                    *TargetActor->GetName(), SkillData->DifficultyClass, SaveRollResult.RawRoll, SaveRollResult.Bonus, SaveRollResult.Total, FinalDamage);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s] Saving Throw FAILED vs DC %d (d20:%d + Bonus:%d = %d) -> Full Damage: %d"),
                    *TargetActor->GetName(), SkillData->DifficultyClass, SaveRollResult.RawRoll, SaveRollResult.Bonus, SaveRollResult.Total, FinalDamage);
            }
        }
        // B-2. 명중 굴림 판정 (Attack Roll vs AC)
        else if (SkillData->ExecutionType == ESkillExecutionType::AttackRoll)
        {
            int32 AttackerBonus = 0;
            if (ICombatStatInterface* StatOwner = Cast<ICombatStatInterface>(Owner))
            {
                AttackerBonus = StatOwner->GetAttackBonus(EWeaponRangeType::Spell);
            }

            int32 TargetAC = 10;
            if (IDamageableInterface* DamageableTarget = Cast<IDamageableInterface>(TargetActor))
            {
                TargetAC = DamageableTarget->GetArmorClass();
            }

            FD20RollResult AttackRoll = UDiceManager::RollD20(AttackerBonus);
            bool bHit = UDiceManager::CheckHit(AttackRoll, TargetAC);

            if (!bHit)
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s] Spell Attack Roll MISSED vs AC %d (d20:%d + Bonus:%d = %d)"),
                    Owner ? *Owner->GetName() : TEXT("None"), TargetAC, AttackRoll.RawRoll, AttackRoll.Bonus, AttackRoll.Total);
                return; // 빗맞으면 데미지 0
            }
        }

        // B-3. 최종 데미지 적용
        if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(TargetActor))
        {
            Damageable->ApplyDamageAmount(FinalDamage, Owner);
        }
    }
}

void UAbilityComponent::ApplySkillEffectToArea(const FSkillDataRow* SkillData, FVector CenterLocation)
{
    if (!SkillData) return;

    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> IgnoreActors;

    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        CenterLocation,
        SkillData->AoERadius,
        ObjectTypes,
        AActor::StaticClass(),
        IgnoreActors,
        OverlappedActors
    );

    if (bHit)
    {
        for (AActor* Target : OverlappedActors)
        {
            ApplySkillEffectToTarget(SkillData, Target);
        }
    }
}

void UAbilityComponent::SpawnSkillImpactVFX(const FSkillDataRow* SkillData, FVector SpawnLocation)
{
    if (!SkillData || !GetWorld()) return;

    // 1. 나이아가라 이펙트가 지정되어 있으면 최우선 스폰
    if (SkillData->ImpactNiagaraVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SkillData->ImpactNiagaraVFX,
            SpawnLocation
        );
    }
    // 2. 나이아가라가 없고 캐스케이드 파티클만 지정되어 있으면 캐스케이드 스폰
    else if (SkillData->ImpactVFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            SkillData->ImpactVFX,
            SpawnLocation
        );
    }
}
