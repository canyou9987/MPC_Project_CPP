// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnManagerSubsystem.h"
#include "SelectableInterface.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DiceManager.h"
#include "GameTimeSubsystem.h"
//#include "Kismet/KismetSystemLibrary.h"

void UTurnManagerSubsystem::StartCombatInRange(FVector CenterLocation, float CombatRadius)
{
	if (bIsCombatActive) return;

	TArray<AActor*> DetectedActors;

	if (UWorld* World = GetWorld())
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->Implements<UTurnParticipantInterface>())
			{
				ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(Actor);
				if (Participant && Participant->GetCurrentHP() > 0)
				{
					float Distance = FVector::Distance(CenterLocation, Actor->GetActorLocation());
					if (Distance <= CombatRadius)
					{
						DetectedActors.Add(Actor);
					}
				}
			}
		}
	}

	// [디버그 로그 추가] 몇 마리가 감지되었는지 확인!
	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Detected Actors Count in Range: %d"), DetectedActors.Num());

	if (DetectedActors.Num() > 0)
	{
		StartCombatWithActors(DetectedActors);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TurnManager] No participants found within radius %.0f! Combat failed to start."), CombatRadius);
	}
}

void UTurnManagerSubsystem::StartCombatWithActors(const TArray<AActor*>& Candidates)
{
	//UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Entering StartCombatWithActors... Candidates: %d"), Candidates.Num());

	if (Candidates.Num() == 0) return;

	bIsCombatActive = true;
	CurrentRound = 1;
	CurrentTurnIndex = 0;

	// 1. 우선권 굴림 및 정렬
	RollInitiativeAndSort(Candidates);

	// 2. 대기열 검사
	if (TurnOrderQueue.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[TurnManager] TurnOrderQueue is 0 after RollInitiativeAndSort!"));
		bIsCombatActive = false;
		return;
	}

	// 3. 모든 참여 유닛 초기화
	for (AActor* Actor : TurnOrderQueue)
	{
		if (IsValid(Actor))
		{
			if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(Actor))
			{
				Selectable->SetSelected(false);
			}

			if (ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(Actor))
			{
				Participant->ClearCommand();
				Participant->SetTurnActive(false);
			}
		}
	}

	// 4. 우선권 1등 유닛 활성화
	AActor* FirstActor = TurnOrderQueue[0];
	if (IsValid(FirstActor))
	{
		if (ITurnParticipantInterface* FirstParticipant = Cast<ITurnParticipantInterface>(FirstActor))
		{
			FirstParticipant->SetTurnActive(true);
		}
	}

	// 5. 방송 (이 이벤트가 실행되면서 CameraPawn::OnTurnChangedHandler 가 호출되어 SelectedUnits 가 세팅됨)
	OnCombatStateChanged.Broadcast(true);

	AActor* CurrentActor = GetCurrentTurnActor();
	OnTurnChanged.Broadcast(CurrentActor, CurrentRound);

	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Combat Started! First Turn: %s"),
		IsValid(CurrentActor) ? *CurrentActor->GetName() : TEXT("NULL"));
}

void UTurnManagerSubsystem::RollInitiativeAndSort(const TArray<AActor*>& RawCandidates)
{
	TurnOrderQueue.Empty();

	// 우선권 점수를 저장할 구조체/튜플 리스트
	struct FInitiativeEntry
	{
		AActor* Actor;
		int32 Score;
	};

	TArray<FInitiativeEntry> Entries;

	for (AActor* Actor : RawCandidates)
	{
		if (IsValid(Actor))
		{
			if (ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(Actor))
			{
				int32 DexMod = Participant->GetDexterityModifier();	//	민첩 수정치를 가져온다.

				FD20RollResult InitRoll = UDiceManager::RollD20(DexMod);	//	민첩 수정치와 함께 주사위 결과를 저장한다.

				UE_LOG(LogTemp, Log, TEXT("[Initiative] %s rolled d20(%d) + DEX(%d) = Total %d"),
					*Actor->GetName(), InitRoll.RawRoll, InitRoll.Bonus, InitRoll.Total);
				Entries.Add({ Actor,InitRoll.Total });						//	주사위 결과물을 저장한다.
			}
		}
	}

	// 2. 우선권 점수 기준 내림차순 정렬 (높은 점수가 먼저 턴을 받음)
	Entries.Sort([](const FInitiativeEntry& A, const FInitiativeEntry& B) {
		return A.Score > B.Score; // 같을 경우 추후 동점자 처리 가능
		});

	// 3. [핵심!] 정렬된 순서대로 TurnOrderQueue 에 채워 넣기
	for (const FInitiativeEntry& Entry : Entries)
	{
		TurnOrderQueue.Add(Entry.Actor);
	}

	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] RollInitiativeAndSort completed. Queue Size: %d"), TurnOrderQueue.Num());
}

void UTurnManagerSubsystem::NextTurn()
{
	if (!bIsCombatActive || TurnOrderQueue.Num() == 0) return;

	// 1. 이전 턴 캐릭터 정리
	if (TurnOrderQueue.IsValidIndex(CurrentTurnIndex))
	{
		AActor* PrevActor = TurnOrderQueue[CurrentTurnIndex];
		if (IsValid(PrevActor))
		{
			if (ISelectableInterface* PrevSelectable = Cast<ISelectableInterface>(PrevActor))
			{
				PrevSelectable->SetSelected(false);
			}
			if (ITurnParticipantInterface* PrevParticipant = Cast<ITurnParticipantInterface>(PrevActor))
			{
				PrevParticipant->ClearCommand();
				PrevParticipant->SetTurnActive(false);
			}
		}
	}

	// 2. 다음 턴 캐릭터 검색
	int32 LoopCount = 0;
	int32 MaxLoop = TurnOrderQueue.Num();

	do
	{
		CurrentTurnIndex++;
		if (CurrentTurnIndex >= TurnOrderQueue.Num())
		{
			CurrentTurnIndex = 0;
			CurrentRound++;

			// -----------------------------------------------------------------
			// 라운드가 넘어가면 게임 시간을 1라운드(6초) 앞으로 전진!
			// -----------------------------------------------------------------
			if (UWorld* World = GetWorld())
			{
				if (UGameTimeSubsystem* TimeSys = World->GetSubsystem<UGameTimeSubsystem>())
				{
					TimeSys->AdvanceTimeRounds(1); // 시간을 +6초 밀어줌
					UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Round %d Started -> Advanced Game Time by 6.0s"), CurrentRound);
				}
			}
		}

		AActor* CheckActor = TurnOrderQueue[CurrentTurnIndex];
		if (IsValid(CheckActor))
		{
			ITurnParticipantInterface* NextCheck = Cast<ITurnParticipantInterface>(CheckActor);
			if (NextCheck && NextCheck->GetCurrentHP() > 0)
			{
				break; // 생존한 캐릭터를 찾음
			}
		}

		LoopCount++;
		if (LoopCount > MaxLoop)
		{
			EndCombat();
			return;
		}
	} while (true);

	// 3. 다음 턴 캐릭터 활성화
	AActor* NextActor = TurnOrderQueue[CurrentTurnIndex];
	if (IsValid(NextActor))
	{
		if (ITurnParticipantInterface* NextParticipant = Cast<ITurnParticipantInterface>(NextActor))
		{
			NextParticipant->SetTurnActive(true);
		}
		if (ISelectableInterface* NextSelectable = Cast<ISelectableInterface>(NextActor))
		{
			NextSelectable->SetSelected(true);
		}

		OnTurnChanged.Broadcast(NextActor, CurrentRound);
	}
}

void UTurnManagerSubsystem::EndCombat()
{
	bIsCombatActive = false;

	for (AActor* Actor : TurnOrderQueue)
	{
		if (IsValid(Actor))
		{
			if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(Actor))
			{
				Selectable->SetSelected(false);
			}

			if (ITurnParticipantInterface* Participant = Cast<ITurnParticipantInterface>(Actor))
			{
				// 명확하게 탐험 모드 복구 전용 함수 호출!
				Participant->ResetToExplorationState();
			}
		}
	}

	TurnOrderQueue.Empty();
	CurrentTurnIndex = 0;
	CurrentRound = 1;

	OnCombatStateChanged.Broadcast(false);

	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Combat Ended Successfully."));
}

AActor* UTurnManagerSubsystem::GetCurrentTurnActor() const
{
	if (TurnOrderQueue.IsValidIndex(CurrentTurnIndex))
	{
		return TurnOrderQueue[CurrentTurnIndex];
	}
	return nullptr;
}

//
