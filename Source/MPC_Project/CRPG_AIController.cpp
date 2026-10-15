// Fill out your copyright notice in the Description page of Project Settings.


#include "CRPG_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "DamageableInterface.h"
//#include "CRPGDataType.h"
#include "FactionSubsystem.h"
#include "FactionInterface.h" // Faction 읽기용

ACRPG_AIController::ACRPG_AIController()
{
	// AI가 감지 및 이동을 수행하도록 기본 설정
	bAttachToPawn = true;
}

void ACRPG_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

bool ACRPG_AIController::FindAndSetTargetEnemy()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	UFactionSubsystem* FactionSys = World->GetSubsystem<UFactionSubsystem>();
	if (!FactionSys) return false;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return false;

	const float DetectionRadius = 1500.0f; // 15m 탐색 반경
	const FVector MyLoc = MyPawn->GetActorLocation();

	// ==========================================
	// 1. Target Lock (기존 타깃 검증 및 고정)
	// ==========================================
	AActor* ExistingTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKeyName));
	if (ExistingTarget)
	{
		// A. 체력 검사
		bool bIsAlive = true;
		if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(ExistingTarget))
		{
			bIsAlive = (Damageable->GetCurrentHP() > 0);
		}

		// B. 적대 관계 및 거리 검사
		bool bIsStillHostile = (FactionSys->GetAttitude(MyPawn, ExistingTarget) == EFactionAttitude::Hostile);
		float DistSq = FVector::DistSquared(MyLoc, ExistingTarget->GetActorLocation());
		bool bInRange = (DistSq <= FMath::Square(DetectionRadius + 300.0f)); // 약간의 유예거리(18m) 부여

		// 모든 조건 만족 시 기존 타깃 유지!
		if (bIsAlive && bIsStillHostile && bInRange)
		{
			return true;
		}

		// 조건 미달 시 타깃 해제 후 새로 탐색
		BB->ClearValue(TargetActorKeyName);
	}

	// ==========================================
	// 2. Overlap 범위 기반 최적화 탐색
	// ==========================================
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn 채널 검색

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MyPawn);

	// 범위 내 Pawn들만 1차적으로 감지
	bool bFoundAny = UKismetSystemLibrary::SphereOverlapActors(
		World,
		MyLoc,
		DetectionRadius,
		ObjectTypes,
		AActor::StaticClass(),
		ActorsToIgnore,
		OverlappedActors
	);

	AActor* ClosestHostileEnemy = nullptr;
	float ClosestDistSq = FLT_MAX;

	if (bFoundAny)
	{
		for (AActor* Candidate : OverlappedActors)
		{
			if (!Candidate) continue;

			// 체력 0 이하 제외
			if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(Candidate))
			{
				if (Damageable->GetCurrentHP() <= 0) continue;
			}

			// FactionSubsystem 기반 'Hostile' 판정
			if (FactionSys->GetAttitude(MyPawn, Candidate) == EFactionAttitude::Hostile)
			{
				float DistSq = FVector::DistSquared(MyLoc, Candidate->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					ClosestHostileEnemy = Candidate;
				}
			}
		}
	}

	// ==========================================
	// 3. 블랙보드 결과 기록
	// ==========================================
	if (ClosestHostileEnemy)
	{
		BB->SetValueAsObject(TargetActorKeyName, ClosestHostileEnemy);
		return true;
	}

	BB->ClearValue(TargetActorKeyName);
	return false;
}
