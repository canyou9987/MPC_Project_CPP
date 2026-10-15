// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveDoor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AInteractiveDoor::AInteractiveDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	RootComponent = DoorFrameMesh;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorFrameMesh);

	// 기본 콜리전 프로필을 BlockAllDynamic으로 기본 지정
	DoorMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	// 문 기본 데이터 초기화
	InteractionInfo.InteractionType = EActionTargetType::Interactable;
	InteractionInfo.bRequiresSkillCheck = false;
	InteractionInfo.DifficultyClass = 10;
	InteractionInfo.InteractionRange = 150.0f;
	InteractionInfo.bIsAlreadyInteracted = false;
}

// Called when the game starts or when spawned
void AInteractiveDoor::BeginPlay()
{
	Super::BeginPlay();

	// 회전 기준값 저장
	ClosedRotation = DoorMesh->GetRelativeRotation();
	OpenedRotation = ClosedRotation + FRotator(0.0f, OpenAngle, 0.0f);

	// 잠긴 문일 경우 Skill Check 요구
	if (bIsLocked)
	{
		InteractionInfo.bRequiresSkillCheck = true;
	}

	SetDoorState(bIsOpen);
}

bool AInteractiveDoor::CanInteract(AActor* Interactor) const
{
	// 플레이어의 접근 이동 자체는 항상 허용
	return true;
}

bool AInteractiveDoor::OnInteract(AActor* Interactor)
{
	// 잠긴 문을 열었거나, 이미 열쇠/Skill Check를 통과한 경우
	if (bIsLocked)
	{
		bIsLocked = false;
		InteractionInfo.bRequiresSkillCheck = false;
		UE_LOG(LogTemp, Warning, TEXT("[Door] %s unlocked the door!"), Interactor ? *Interactor->GetName() : TEXT("None"));
	}

	// 토글 형태로 문 여닫기 실행
	SetDoorState(!bIsOpen);

	UE_LOG(LogTemp, Warning, TEXT("[Door] %s toggled the door. New State -> IsOpen: %s"),
		Interactor ? *Interactor->GetName() : TEXT("None"), bIsOpen ? TEXT("True") : TEXT("False"));

	return true;
}

FText AInteractiveDoor::GetInteractionPrompt() const
{
	if (bIsLocked)
	{
		return FText::Format(
			FText::FromString(TEXT("Open Locked Door (DC {0})")),
			FText::AsNumber(InteractionInfo.DifficultyClass)
		);
	}

	if (bIsOpen)
	{
		return FText::FromString(TEXT("Close Door"));
	}

	return FText::FromString(TEXT("Open Door"));
}

void AInteractiveDoor::SetDoorState(bool bOpen)
{
	bIsOpen = bOpen;

	if (bIsOpen)
	{
		DoorMesh->SetRelativeRotation(OpenedRotation);
		// 문이 열리면 통과 가능하도록 콜리전 무효화 또는 NavMesh 개방
		DoorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	else
	{
		DoorMesh->SetRelativeRotation(ClosedRotation);
		// 문이 닫히면 다시 막힘
		DoorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}



