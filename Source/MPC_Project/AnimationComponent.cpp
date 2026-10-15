// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimationComponent.h"
#include "GameFramework/Character.h"
#include "AnimStateInterface.h"
#include "EquipmentComponent.h"

// Sets default values for this component's properties
UAnimationComponent::UAnimationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		// =========================================================
		// [추가] EquipmentComponent 델리게이트 구독
		// =========================================================
		if (UEquipmentComponent* EquipComp = Owner->FindComponentByClass<UEquipmentComponent>())
		{
			EquipComp->OnWeaponDataChanged.AddUObject(this, &UAnimationComponent::HandleWeaponDataChanged);

			// 초기화 직후 이미 설정된 무기 애니메이션 데이터가 있다면 즉시 반영
			HandleWeaponDataChanged(EquipComp->GetCurrentWeaponData());
		}
	}
	
}


// Called every frame
void UAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAnimationComponent::SetAnimState(ECharacterAnimState NewState)
{
	if (CurrentAnimState == ECharacterAnimState::Dead) return;
	CurrentAnimState = NewState;
}

void UAnimationComponent::SetWeaponOverlayType(EWeaponOverlayType NewOverlay)
{
	CurrentWeaponOverlay = NewOverlay;
}

float UAnimationComponent::PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName)
{
	if (!MontageToPlay || !OwnerActor) return 0.0f;

	ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
	if (OwnerChar)
	{
		return OwnerChar->PlayAnimMontage(MontageToPlay, InPlayRate, StartSectionName);
	}

	return 0.0f;
}

float UAnimationComponent::PlayWeaponAttackMontage(UAnimMontage* DefaultMontageFallback)
{
	// Get()을 사용하여 TObjectPtr을 UAnimMontage* raw pointer로 명시적 변환!
	UAnimMontage* TargetMontage = CurrentWeaponAttackMontage.Get() ? CurrentWeaponAttackMontage.Get() : DefaultMontageFallback;

	if (TargetMontage)
	{
		return PlayMontage(TargetMontage);
	}
	return 0.0f;
}

void UAnimationComponent::SetCurrentWeaponAnimData(UAnimMontage* AttackMontage, EWeaponOverlayType OverlayType)
{
	CurrentWeaponAttackMontage = AttackMontage;
	CurrentWeaponOverlay = OverlayType;
}

//	PlayWeaponAttackMontage와 동일하게 PlayMontage로 넘겨서 처리!
float UAnimationComponent::PlayCastMontage(UAnimMontage* CastMontage, float PlayRate)
{
	if (!CastMontage) return 0.0f;

	return PlayMontage(CastMontage, PlayRate);
}

void UAnimationComponent::StopCurrentMontage(float BlendOutTime)
{
	if (!OwnerActor) return;

	// IAnimStateInterface 또는 ACharacter를 통해 현재 몽타주 정지
	if (ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor))
	{
		if (UAnimInstance* AnimInst = OwnerChar->GetMesh() ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr)
		{
			// 현재 재생 중인 몽타주를 지정한 BlendOutTime 동안 부드럽게 정지
			AnimInst->Montage_Stop(BlendOutTime);
		}
	}
}

void UAnimationComponent::HandleWeaponDataChanged(const FWeaponDataRow& NewWeaponData)
{
	// 기존에 가지고 계신 무기 애니메이션 세팅 함수 호출
	SetCurrentWeaponAnimData(
		NewWeaponData.WeaponAttackMontage,
		NewWeaponData.WeaponOverlayType
	);

	UE_LOG(LogTemp, Warning, TEXT("[%s][AnimationComponent] Anim Overlay Synced via Delegate: OverlayType = %d"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		(int32)NewWeaponData.WeaponOverlayType);
}