// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DamageableInterface.h"

// Sets default values
AProjectileActor::AProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌체 생성 및 설정
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionComp;

	// 2. 메시 컴포넌트 생성 (충돌은 하지 않도록 설정)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. ProjectileMovement 세팅 (타깃을 향해 날아가는 추적 모드 활성화)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 이동 방향으로 화살 회전
	ProjectileMovement->bIsHomingProjectile = true;       // 타깃 추적 모드
	ProjectileMovement->HomingAccelerationMagnitude = 15000.0f;

	InitialLifeSpan = 5.0f; // 5초 후 자동 소멸
}


// Called when the game starts or when spawned
void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileActor::OnOverlap);
	
}

// Called every frame
void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AProjectileActor::InitProjectile(AActor* InInstigator, AActor* InTarget, bool bInIsHit, int32 InDamage)
{
	SetOwner(InInstigator);
	TargetActor = InTarget;
	bIsHit = bInIsHit;
	DamageAmount = InDamage;

	// 타깃이 존재하면 이동 컴포넌트에 Homing Target으로 지정
	if (TargetActor && ProjectileMovement)
	{
		ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
	}
}

void AProjectileActor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 충돌한 대상이 '내가 노린 목표 대상(TargetActor)'일 경우에만 처리
	if (OtherActor && OtherActor == TargetActor)
	{
		// 주사위 판정이 명중(Hit)이었던 경우 피해 적용
		if (bIsHit)
		{
			if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(OtherActor))
			{
				Damageable->ApplyDamageAmount(DamageAmount, GetOwner());
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Projectile Missed Target!"), *GetName());
		}

		// 피격 판정 후 발사체 소멸
		Destroy();
	}
}

