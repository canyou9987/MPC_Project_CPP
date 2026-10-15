// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionEconomyInterface.h" // 인터페이스 포함
#include "ActionEconomyComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPC_PROJECT_API UActionEconomyComponent : public UActorComponent , public IActionEconomyInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionEconomyComponent();

protected:		
	// Called when the game starts
	virtual void BeginPlay() override;

public: //	함수들 들오는 자리

	// GameTimeSubsystem의 OnRoundTick (휴식 등 강제 시간 경과 시 호출)
	UFUNCTION()
	void OnGameRoundTick(int32 TotalRoundsPassed);

	// 턴 시작 시 자원 초기화
	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	void ResetActionEconomy();

	// 자원 소모 검사 및 차감 함수들
	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	bool ConsumeStandardAction();

	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	bool ConsumeMoveAction();

	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	bool ConsumeSwiftAction();

	UFUNCTION(BlueprintCallable, Category = "Action Economy")
	bool ConsumeMoveDistance(float Distance);

	// Getter
	//bool HasStandardAction() const { return bHasStandardAction; }
	//bool HasMoveAction() const { return bHasMoveAction; }
	//bool HasSwiftAction() const { return bHasSwiftAction; }
	//float GetRemainingMoveDistance() const { return RemainingMoveDistance; }
	// 자원이 하나라도 소모되었는지 검사하는 함수
	//bool IsActionEconomyConsumed() const;

public : 
	//	IActionEconomyInterface 구현
	// Getter
	virtual bool HasStandardAction() const override { return bHasStandardAction; }
	virtual bool HasMoveAction() const override { return bHasMoveAction; }
	virtual bool HasSwiftAction() const override { return bHasSwiftAction; }
	virtual float GetRemainingMoveDistance() const override { return RemainingMoveDistance; }
	virtual float GetMaxMoveDistance() const override { return MaxMoveDistance; }
	virtual FOnActionEconomyChanged* GetOnActionEconomyChangedDelegate() override { return &OnActionEconomyChanged; }

protected:
	// 탐험 모드 행동력 소모 시 6.0초 회복 타이머 시작
	void StartExplorationCooldownTimer();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 턴당 최대 이동거리 (패스파인더 기준 30ft = 약 900cm / 9m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Economy")
	float MaxMoveDistance = 900.0f;

	// 이번 턴 남은 이동거리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Economy")
	float RemainingMoveDistance = 900.0f;

	// 행동 자원 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Economy")
	bool bHasStandardAction = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Economy")
	bool bHasMoveAction = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Economy")
	bool bHasSwiftAction = true;

	UPROPERTY(BlueprintAssignable, Category = "ActionEconomy")
	FOnActionEconomyChanged OnActionEconomyChanged;

	// 개별 6초 회복용 타이머 핸들
	FTimerHandle CooldownTimerHandle;
};
