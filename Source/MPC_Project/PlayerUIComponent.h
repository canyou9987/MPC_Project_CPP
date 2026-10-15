// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerUIComponent.generated.h"

//	전방 선언!!!
class UCRPGMainHUDWidget;
class UInventoryWindowWidget;
class APlayerController;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPC_PROJECT_API UPlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UPlayerUIComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// CameraPawn에서 생성된 PlayerController 및 MainHUDWidget 포인터 초기화
	void InitializePlayerUI(APlayerController* InPC, UCRPGMainHUDWidget* InHUDWidget);

	// I키 입력 시 실행할 토글 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventoryUI();

	// 외부(CameraPawn 등)에서 현재 선택된 액터를 UI컴포넌트에 전달할 때 사용
	// 선택된 액터 변경 통지 (InTargetActor가 nullptr이면 인벤토리/HUD 해제)
	void SetSelectedTargetActor(AActor* InTargetActor);

protected:
	// 마우스 밑 호버링 타깃 감지 및 3D->2D 스크린 프롬프트 UI 갱신
	void UpdateHoveredTargetUI();

protected:
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<UCRPGMainHUDWidget> MainHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInventoryWindowWidget> InventoryWindowWidgetClass;

	UPROPERTY()
	TObjectPtr<UInventoryWindowWidget> InventoryWindowWidget;

	// 현재 선택된 액터 (인터페이스 추적용)
	UPROPERTY()
	TObjectPtr<AActor> SelectedTargetActor;

};
