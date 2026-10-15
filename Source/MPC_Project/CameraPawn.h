// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActionTargetType.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

class UCRPGMainHUDWidget;

// 헤더 상단 클래스 전방 선언 영역에 추가
class UPlayerUIComponent;

UCLASS()
class MPC_PROJECT_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//----------------------------------------------------------------------------------------------
protected:	// IMC 선언
	 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")	//	IA 집합체
	TObjectPtr<UInputMappingContext> IMC_Move;

protected:	//	IA 선언
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")	//	WASD 카메라	이동
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")	//	EQ 카메라 회전
	TObjectPtr<UInputAction> IA_Rotate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")	//	마우스 휠 업,다운 : 줌아웃,줌인
	TObjectPtr<UInputAction> IA_Zoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Select;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_RightClick;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleHold;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleAutoCombat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_StartCombat;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_EndTurn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_DebugBuff;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_DebugSkill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleInventory;

public:	//	컴포넌트
	UCameraComponent* Camera;			//	카메라
	USpringArmComponent* SpringArm;		//	스프링암
	//USceneComponent* Sence;				//	씬컴포넌트 : RootComponent 사용하는 중이므로 사용X

public:	//	변수값
	UPROPERTY(EditAnywhere)
	float ArmLenght = 1200.0f;											//	카메라 암 길이	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MoveSpeed = 1000.0f;											//	이동속도
	UPROPERTY(EditDefaultsOnly, Category = "Camera Settings")
	FRotator CameraDefaultRotator;										//	카메라 앵글 각도

	FVector LastRightClickLocation = FVector::ZeroVector;
	//-----------------------------------------------------------------------------
	//	카메라 회전 및 줌인 세팅값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float RotateSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomStep = 300.0f; // 휠 1스텝당 이동할 목표 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomInterpSpeed = 10.0f; // 줌 보간 속도 (높을수록 빠르게 반응)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smoothing")
	float MoveInterpSpeed = 150.0f;	//	이동 보관 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smoothing")
	float RotateInterpSpeed = 12.0f;	//	회전 보간 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MinArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MaxArmLength = 2500.0f;

	// 최종 목표 줌 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float DesiredArmLength;

	FRotator DesiredRotation;		//	보정 목표 로테이션 값

	FVector DesiredVector;			//	보정 목표 로케이션 값

	//------------------------------------------------------------------------------
	//	화면 가장자리 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|EdgeScroll")
	bool bEnableEdgeScroll = true;

	// 감지할 화면 가장자리 여백 (픽셀 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|EdgeScroll")
	float EdgeMargin = 15.0f;

	//------------------------------------------------------------------
	// 인터페이스 관련 변수
	// 현재 선택된 대상 (인터페이스 포인터 타입)
	class ISelectableInterface* CurrentSelectedTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	TArray<TScriptInterface<ISelectableInterface>> SelectedUnits;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	AActor* CurrentlyControlledCharacter;

	// 에디터에서 만든 DT_BuffList를 실행시켜줄  소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Buff")
	FDataTableRowHandle DebugBuffRow;

	// 디버그 테스트용 스킬 RowHandle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Ability")
	FDataTableRowHandle DebugSkillRow;

	//------------------------------------------------------------------
	//	위젯클래스 관련 변수

	// 에디터(BP)에서 WBP_MainHUD 클래스를 지정받을 변수
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCRPGMainHUDWidget> MainHUDWidgetClass;

	// 생성된 HUD 인스턴스 보관 변수
	UPROPERTY()
	TObjectPtr<class UCRPGMainHUDWidget> MainHUDWidget;

	// 드래그 박스 관련 변수
	bool bIsDragging = false;
	FVector2D DragStartScreenPosition;
	FVector2D DragCurrentScreenPosition;

	//	레지스트리 변수
	//	선택 가능한 유닛들의 포인터를 모아두는 레지스트리 (포인터 안전을 위해 TWeakObjectPtr 사용)
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SelectableUnitRegistry;

protected:
	void Move(const FInputActionValue&);								//	카메라 이동 함수
	void RotateCamera(const FInputActionValue& Value);					//	카메라 회전 함수
	void ZoomCamera(const FInputActionValue& Value);					//	카메라 줌인 함수
	void HandleEdgeScroll(float DeltaTime);								//	화면 가장자리 이동 로직

	void DeselectAllUnits();											//	전체 선택 해제
	void SelectUnit(ISelectableInterface* UnitToSelect);				//	전체 선택
	void DeselectUnit(ISelectableInterface* UnitToDeselect);			//	선택 해제

	void OnSelectPressed();												//	왼쪽 마우스 클릭시 호출되는 함수
	void OnRightClickPressed();											//	오른쪽 마우스 클릭시 호출되는 함수

	void OnToggleHoldPressed();
	void OnToggleAutoCombatPressed();

	// I키 입력 시 실행할 인벤토리 토글 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventoryUI();

	//EActionTargetType DetermineTargetType(const FHitResult& HitResult, AActor*& OutTargetActor) const;	// 마우스 라인트레이스 결과(HitResult)를 바탕으로 타겟 유형을 판별하는 함수

	// --- 턴 제어 명령 함수 ---

	// 전투 시작 요청 (예: Enter 키 또는 전투 진입 버튼)
	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void OnStartCombatPressed();
	// 턴 넘기기 요청 (예: SpaceBar 키 또는 턴 종료 UI 버튼)
	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void OnEndTurnPressed();

	// P키 토글용 함수
	UFUNCTION(BlueprintCallable, Category = "TurnSystem")
	void OnToggleCombatPressed();

	UFUNCTION()
	void OnTurnChangedHandler(AActor* NewActiveActor, int32 CurrentRound);

	// B키 눌렀을 때 실행할 함수
	void OnDebugApplyBuffPressed();

	// 디버그 스킬 시전 테스트 함수 (N키 입력 등)
	UFUNCTION(BlueprintCallable, Category = "Debug|Ability")
	void OnDebugCastSkillPressed();

	// 마우스 커서가 UMG UI(버튼, 팝업창 등) 위에 있는지 판별하는 함수
	bool IsPointerOverUI() const;

	// 드래그 박스 만들기
	void StartSelectionDrag();				// 드래그 시작
	void EndSelectionDrag();				// 종료
	void SelectUnitsInRectangle();			// 선택 처리 함수

	// 선택 가능한 유닛 등록/해제 함수 (Register)
	void RegisterSelectableUnit(AActor* UnitToRegister);
	void UnregisterSelectableUnit(AActor* UnitToUnregister);

public:
	// PlayerUIComponent 게터
	UFUNCTION(BlueprintCallable, Category = "Components")
	UPlayerUIComponent* GetPlayerUIComponent() const { return PlayerUIComponent; }

	// HUD 사각형 그리기를 위한 겟터
	bool IsDragging() const { return bIsDragging; }
	FVector2D GetDragStartPos() const { return DragStartScreenPosition; }
	FVector2D GetDragCurrentPos() const { return DragCurrentScreenPosition; }

protected:
	// 상호작용 UI 및 Overlay 전담 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlayerUIComponent> PlayerUIComponent;
};
