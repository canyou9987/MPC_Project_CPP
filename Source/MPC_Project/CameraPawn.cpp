// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"
#include "SelectableInterface.h"
//#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "InteractableInterface.h"
#include "TurnManagerSubsystem.h"
#include "EngineUtils.h"
#include "BuffableInterface.h"
//#include "AbilityComponent.h"
#include "SkillCasterInterface.h"
#include "CRPGMainHUDWidget.h" // HUD 위젯 헤더 추가!
#include "Blueprint/UserWidget.h" // CreateWidget 사용을 위해 필요
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "GameFramework/PlayerController.h"
#include "PlayerUIComponent.h"
#include "InventoryHolderInterface.h"
//#include "InventoryComponent.h"
#include "GameFramework/HUD.h"
#include "UnitRegistrySubsystem.h"
#include "FlowFieldSubsystem.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//	컴포넌트 생성
	Camera = CreateDefaultSubobject< UCameraComponent>("Camera");
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");

	//	변수값 생성
	CameraDefaultRotator = FRotator(-60.0f, 0.0f, 0.0f);
	DesiredRotation = CameraDefaultRotator;

	//	부착
	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm, SpringArm->GetAttachSocketName());

	//	스프링암 세팅
	SpringArm->bDoCollisionTest = false;
	SpringArm->TargetArmLength = ArmLenght;
	SpringArm->SetRelativeRotation(CameraDefaultRotator);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// PlayerUIComponent 생성
	PlayerUIComponent = CreateDefaultSubobject<UPlayerUIComponent>(TEXT("PlayerUIComponent"));

}
// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{

	Super::BeginPlay();

	// TurnManagerSubsystem의 턴 변경 이벤트 바인딩
	if (UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		TurnSys->OnTurnChanged.AddDynamic(this, &ACameraPawn::OnTurnChangedHandler);
	}

	//	PC타입 컨트롤러를 가져옴
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{		//	컨트롤러 인풋에서 서브시스템을 가져옴
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			PC->bShowMouseCursor = true;       // 마우스 커서 보이기
			PC->bEnableClickEvents = true;     // 클릭 이벤트 활성화
			PC->bEnableMouseOverEvents = true; // 마우스 오버 이벤트 활성화

			FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways); // 뷰포트에 항상 가둠
		InputMode.SetHideCursorDuringCapture(false); // 마우스 클릭 시 커서가 숨겨지지 않도록 함

		PC->SetInputMode(InputMode);
			if (IMC_Move)	//	서브 시스템에서 IMC를 가져옴
			{
				Subsystem->AddMappingContext(IMC_Move, 0);
			}
		}
	}

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (MainHUDWidgetClass && PC)
	{
		MainHUDWidget = CreateWidget<UCRPGMainHUDWidget>(GetWorld(), MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();

			// PlayerUIComponent에 PlayerController와 HUD 포인터 전달
			if (PlayerUIComponent)
			{
				PlayerUIComponent->InitializePlayerUI(PC, MainHUDWidget);
			}
		}
	}

	if (SpringArm)
	{
		DesiredArmLength = SpringArm->TargetArmLength;
	}
}
// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 드래그 중일 때 매 프레임 마우스의 현재 화면 좌표 갱신
	if (bIsDragging)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->GetMousePosition(DragCurrentScreenPosition.X, DragCurrentScreenPosition.Y);
		}
	}

	// 카메라 회전 부드럽게 보간 (RInterpTo)
	FRotator CurrentRotation = GetActorRotation();
	FVector CurrenntLocation = GetActorLocation();

	SetActorRotation(FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, RotateInterpSpeed));

	if (SpringArm)
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredArmLength, DeltaTime, ZoomInterpSpeed);
	}

	if (bEnableEdgeScroll)
	{
		HandleEdgeScroll(DeltaTime);
	}

	// UI 업데이트 부분은 UPlayerUIComponent가 전담하므로 여기 있던 코드는 삭제!
}
// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move)
		{
			// 키를 누르고 있는 동안 매 프레임 계속 실행되도록 Triggered 사용
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ACameraPawn::Move);
		}
		if (IA_Rotate)	//	IA 카메라 회전
		{
			EnhancedInputComponent->BindAction(IA_Rotate, ETriggerEvent::Triggered, this, &ACameraPawn::RotateCamera);
		}
		if (IA_Zoom)	//	IA 카메라 줌인
		{
			EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ACameraPawn::ZoomCamera);
		}
		if (IA_Select)	//	IA 선택
		{
			// 1. 마우스 누를 때: 드래그 시작 좌표 저장
			EnhancedInputComponent->BindAction(IA_Select, ETriggerEvent::Started, this, &ACameraPawn::StartSelectionDrag);

			// 2. 마우스 뗄 때: 이동 거리에 따라 '드래그 범위 선택' 또는 '기존 OnSelectPressed' 분기 실행
			EnhancedInputComponent->BindAction(IA_Select, ETriggerEvent::Completed, this, &ACameraPawn::EndSelectionDrag);
		}
		if (IA_RightClick)
		{
			EnhancedInputComponent->BindAction(IA_RightClick, ETriggerEvent::Started, this, &ACameraPawn::OnRightClickPressed);
		}

		// Hold 및 자동 전투 키 바인딩
		if (IA_ToggleHold)
		{
			EnhancedInputComponent->BindAction(IA_ToggleHold, ETriggerEvent::Started, this, &ACameraPawn::OnToggleHoldPressed);
		}
		if (IA_ToggleAutoCombat)
		{
			EnhancedInputComponent->BindAction(IA_ToggleAutoCombat, ETriggerEvent::Started, this, &ACameraPawn::OnToggleAutoCombatPressed);
		}

		//	턴제 전투 시작 바인딩
		if (IA_StartCombat)
		{
			EnhancedInputComponent->BindAction(IA_StartCombat, ETriggerEvent::Started, this, &ACameraPawn::OnToggleCombatPressed);
		}
		//	턴 넘기기 바인딩
		if (IA_EndTurn)
		{
			EnhancedInputComponent->BindAction(IA_EndTurn, ETriggerEvent::Started, this, &ACameraPawn::OnEndTurnPressed);
		}

		if (IA_DebugBuff)
		{
			EnhancedInputComponent->BindAction(IA_DebugBuff, ETriggerEvent::Started, this, &ACameraPawn::OnDebugApplyBuffPressed);
		}

		if (IA_DebugSkill)
		{
			EnhancedInputComponent->BindAction(IA_DebugSkill, ETriggerEvent::Started, this, &ACameraPawn::OnDebugCastSkillPressed);
		}

		// I키 입력 시 인벤토리 창 토글

		if (IA_ToggleInventory)
		{
			EnhancedInputComponent->BindAction(IA_ToggleInventory, ETriggerEvent::Started, this, &ACameraPawn::ToggleInventoryUI);
		}

	}
}
void ACameraPawn::Move(const FInputActionValue& Value)
{
	//	IA_MOVE에서 Vector2D값을 가져옴
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (MovementVector.IsZero())
	{
		return;
	}

	//	월드에서 독립적인 델타 타임을 가져옴
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	//	카메라의 전방/우측 방향 이동
	FVector Foward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	Foward.Z = 0.0f;
	Foward.Normalize();

	Right.Z = 0.0f;
	Right.Normalize();

	FVector MoveDirection = (Foward * MovementVector.Y + Right * MovementVector.X).GetSafeNormal();
	AddActorWorldOffset(MoveDirection * MoveSpeed * DeltaTime);	

	//DesiredLocaction = GetActorLocation();
	//DesiredLocaction += Foward * MovementVector.Y * MoveSpeed * DeltaTime;
	//DesiredLocaction += Right * MovementVector.X * MoveSpeed * DeltaTime;

	//NewLocation = ((Foward * MovementVector.Y) + (Right * MovementVector.X) * MoveSpeed * DeltaTime);
	//SetActorLocation(NewLocation);
}
void ACameraPawn::RotateCamera(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();				//	IA 인풋 값에서 플로트 값을 가져옴
	if (FMath::IsNearlyZero(InputValue)) return;		//	만약 입력값이 0이 된다면 움직이질않음

	float DeltaTime = GetWorld()->GetDeltaSeconds();	//	델타 타임을 가져옴

	// 액터 자체의 Yaw 회전값을 변경하여 카메라 붐과 카메라 전체 시점을 돌립니다.	액터의 앞백터랑 오른쪽 백터가 향하는값이 달라짐
	//AddActorLocalRotation(FRotator(0.0f, InputValue * RotateSpeed * DeltaTime, 0.0f));	//	로컬기준이면 찐빠남 안됨
	//AddActorWorldRotation(FRotator(0.0f, InputValue * RotateSpeed * DeltaTime, 0.0f));	//	월드기준으로 만들었지만 보간을 위해 여기서 하지않음
	
	// 목표 Yaw 값 업데이트
	DesiredRotation.Yaw += InputValue * RotateSpeed * DeltaTime;
}
void ACameraPawn::ZoomCamera(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue)) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();	//	델타 타임을 가져옴

	if (SpringArm)
	{
		// 휠을 올리면(Positive) 거리 감소(Zoom In), 내리면(Negative) 거리 증가(Zoom Out)
		DesiredArmLength = SpringArm->TargetArmLength - (InputValue * ZoomStep);

		// 최소/최대 거리 제한 (Clamping)
		DesiredArmLength = FMath::Clamp(DesiredArmLength, MinArmLength, MaxArmLength);
		//SpringArm->TargetArmLength = FMath::FInterpTo(NewArmLength, MinArmLength, MaxArmLength, DeltaTime);
	}
}
void ACameraPawn::HandleEdgeScroll(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 1. 현재 뷰포트(화면) 크기 가져오기
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	if (ViewportSizeX <= 0 || ViewportSizeY <= 0) return;

	// 2. 현재 마우스 커서의 화면 좌표 가져오기
	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY)) return;

	FVector2D ScrollInput(0.0f, 0.0f);

	// 3. 마우스가 경계선(Margin) 내에 들어왔는지 체크
	// X축 (좌 / 우)
	if (MouseX <= EdgeMargin)
	{
		ScrollInput.X = -1.0f; // 좌측 이동
	}
	else if (MouseX >= ViewportSizeX - EdgeMargin)
	{
		ScrollInput.X = 1.0f;  // 우측 이동
	}

	// Y축 (상 / 하)
	if (MouseY <= EdgeMargin)
	{
		ScrollInput.Y = 1.0f;  // 전진 (화면 위쪽)
	}
	else if (MouseY >= ViewportSizeY - EdgeMargin)
	{
		ScrollInput.Y = -1.0f; // 후진 (화면 아래쪽)
	}

	// 입력이 없으면 리턴
	if (ScrollInput.IsZero()) return;

	// 4. 기존 Move 로직과 동일하게 지면 기준 이동 벡터 적용
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();
	Forward.Z = 0.0f; Forward.Normalize();
	Right.Z = 0.0f; Right.Normalize();

	FVector MoveDirection = (Forward * ScrollInput.Y + Right * ScrollInput.X).GetSafeNormal();	//	광역 변수화 시키기
	AddActorWorldOffset(MoveDirection * MoveSpeed * DeltaTime);									

	//DesiredLocaction += MoveDirection * MoveSpeed * DeltaTime;
	
}
void ACameraPawn::DeselectAllUnits()
{
	for (int32 i = SelectedUnits.Num() - 1; i >= 0; --i)
	{
		if (SelectedUnits[i].GetInterface())
		{
			SelectedUnits[i]->SetSelected(false);
		}
	}
	SelectedUnits.Reset(); // Empty() 대신 메모리까지 완벽하게 비워주는 Reset() 사용

	// 선택 해제 시 PlayerUIComponent 및 MainHUD 통지
	if (PlayerUIComponent) PlayerUIComponent->SetSelectedTargetActor(nullptr);
	if (MainHUDWidget) MainHUDWidget->SetSelectedUnit(nullptr);

}
void ACameraPawn::SelectUnit(ISelectableInterface* UnitToSelect)
{
	if (!UnitToSelect) return;

	// 인터페이스만을 통해 최상위 통제 객체(SquadManager 또는 Solo 유닛) 획득!
	ISelectableInterface* RealTarget = UnitToSelect->GetSelectionOwner();
	if (!RealTarget) return;

	RealTarget->SetSelected(true);
	SelectedUnits.AddUnique(RealTarget->_getUObject());
}
void ACameraPawn::DeselectUnit(ISelectableInterface* UnitToDeselect)
{
	if (!UnitToDeselect) return;

	UnitToDeselect->SetSelected(false);
	SelectedUnits.Remove(UnitToDeselect->_getUObject());
}
void ACameraPawn::OnSelectPressed()
{
	//  UI 위를 클릭한 경우 게임 월드 선택/스킬 취소 로직 실행 안 함!
	if (IsPointerOverUI()) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		ISelectableInterface* Selectable = Cast<ISelectableInterface>(HitActor);

		const bool bIsShiftDown = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift);

		if (Selectable)
		{
			if (bIsShiftDown)
			{
				if (Selectable->IsSelected()) DeselectUnit(Selectable);
				else SelectUnit(Selectable);
			}
			else
			{
				DeselectAllUnits(); // 기존 선택 무조건 깨끗하게 초기화
				SelectUnit(Selectable);

				if (MainHUDWidget && HitActor) MainHUDWidget->SetSelectedUnit(HitActor);
			}
		}
		else
		{
			// 허공/지면 클릭 시 Shift 키를 누르지 않았다면 무조건 전체 선택 해제!
			if (!bIsShiftDown)
			{
				DeselectAllUnits();
			}
		}
	}
}

void ACameraPawn::OnRightClickPressed()
{
	// UI 위를 우클릭한 경우 이동/공격/스킬 시전 명령 스킵!
	if (IsPointerOverUI())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult HitResult;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult)) return;

	AActor* HitActor = HitResult.GetActor();
	const FVector TargetLocation = HitResult.Location;

	// 1. 명령을 내릴 최상위 통제 대상(SquadManager 또는 Solo 유닛) 수집
	TArray<ISelectableInterface*> TargetSelectablesToCommand;
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();

	if (TurnSys && TurnSys->IsCombatActive())
	{
		if (AActor* CurrentTurnActor = TurnSys->GetCurrentTurnActor())
		{
			if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(CurrentTurnActor))
			{
				TargetSelectablesToCommand.Add(Selectable->GetSelectionOwner());
			}
		}
	}
	else
	{
		for (const TScriptInterface<ISelectableInterface>& Unit : SelectedUnits)
		{
			if (ISelectableInterface* Selectable = Unit.GetInterface())
			{
				// GetSelectionOwner()를 거쳐 SquadManager의 인터페이스를 수집!
				ISelectableInterface* OwnerSelectable = Selectable->GetSelectionOwner();
				if (OwnerSelectable)
				{
					TargetSelectablesToCommand.AddUnique(OwnerSelectable);
				}
			}
		}
	}

	if (TargetSelectablesToCommand.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[CameraPawn] No units selected to command."));
		return;
	}

	// =========================================================================
	// CASE 0: 스킬 대기 상태 (HasPendingSkill) 처리
	// =========================================================================
	bool bExecutedSkillCommand = false;

	for (ISelectableInterface* Selectable : TargetSelectablesToCommand)
	{
		if (!Selectable) continue;

		if (ISkillCasterInterface* Caster = Cast<ISkillCasterInterface>(Selectable->_getUObject()))
		{
			if (Caster->HasPendingSkill())
			{
				bExecutedSkillCommand = true;
				bool bSuccess = Caster->ExecutePendingSkill(HitActor, TargetLocation);

				UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] ExecutePendingSkill Result: %s"),
					bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
			}
		}
	}

	if (bExecutedSkillCommand)
	{
		return;
	}

	// =========================================================================
	// CASE A: 상호작용 대상 (Interactable / Loot) 
	// =========================================================================
	if (HitActor)
	{
		if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitActor))
		{
			EActionTargetType TargetType = Interactable->GetActionTargetType();

			UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] Interaction Clicked!: %s (TargetType: %d)"),
				*HitActor->GetName(), (int32)TargetType);

			for (ISelectableInterface* Selectable : TargetSelectablesToCommand)
			{
				if (Selectable)
				{
					Selectable->CommandTargetAction(HitActor, TargetType);
				}
			}
			return; // 상호작용 명령 완료 후 종료
		}
	}

	// =========================================================================
	// CASE B: 적 타깃팅 (Enemy) -> 일반 공격
	// =========================================================================
	ISelectableInterface* TargetSelectable = Cast<ISelectableInterface>(HitActor);
	bool bIsEnemyTarget = false;
	if (TargetSelectable && TargetSelectable->GetActionTargetType() == EActionTargetType::Enemy)
	{
		bIsEnemyTarget = true;
	}

	if (bIsEnemyTarget)
	{
		for (ISelectableInterface* Selectable : TargetSelectablesToCommand)
		{
			if (Selectable)
			{
				Selectable->CommandTargetAction(HitActor, EActionTargetType::Enemy);
			}
		}
		return;
	}

	// =========================================================================
	// CASE C: 지형 클릭 (Ground) -> 이동 명령 하달
	// =========================================================================
	if (FVector::DistSquared(LastRightClickLocation, TargetLocation) > 1600.0f)
	{
		LastRightClickLocation = TargetLocation;

		for (ISelectableInterface* SelectableTarget : TargetSelectablesToCommand)
		{
			if (SelectableTarget)
			{
				// SquadManager인 경우 -> ASquadManager::CommandTargetAction 실행!
				SelectableTarget->CommandTargetAction(EActionTargetType::Ground, TargetLocation);
			}
		}
	}
}


void ACameraPawn::OnToggleHoldPressed()
{
	for (const TScriptInterface<ISelectableInterface>& Unit : SelectedUnits)
	{
		if (Unit)
		{
			// ABaseCharacter를 몰라도 인터페이스 함수로 직접 명령!
			Unit->ToggleHoldPosition();
		}
	}
}

void ACameraPawn::OnToggleAutoCombatPressed()
{
	UE_LOG(LogTemp, Log, TEXT("[CameraPawn] V Key Pressed -> Toggling AutoCombat..."));

	// 1. 선택된 유닛이 있다면 선택된 유닛들만 토글
	if (SelectedUnits.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[CameraPawn] Toggling AutoCombat for Selected Units (Count: %d)"), SelectedUnits.Num());
		for (const TScriptInterface<ISelectableInterface>& Unit : SelectedUnits)
		{
			if (Unit)
			{
				Unit->ToggleAutoCombat();
			}
		}
		return;
	}

	// 2. 선택된 유닛이 없다면 월드의 모든 대상 토글
	UE_LOG(LogTemp, Log, TEXT("[CameraPawn] No units selected. Toggling AutoCombat for ALL units in world..."));
	if (UWorld* World = GetWorld())
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->Implements<UTurnParticipantInterface>())
			{
				if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(Actor))
				{
					Selectable->ToggleAutoCombat();
				}
			}
		}
	}
}

void ACameraPawn::OnStartCombatPressed()
{
	if (UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		if (!TurnSys->IsCombatActive())
		{
			// 현재 카메라 위치를 기준으로 2000cm(20m) 이내의 캐릭터들을 대기열에 등록하고 전투 개시!
			TurnSys->StartCombatInRange(GetActorLocation(), 2000.0f);
			UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] Combat Start Requested!"));
		}
	}
}

void ACameraPawn::OnEndTurnPressed()
{
	if (UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		if (TurnSys->IsCombatActive())
		{
			TurnSys->NextTurn(); // 다음 순서 유닛에게 턴 넘김
		}
	}
}

void ACameraPawn::OnToggleCombatPressed()
{
	UTurnManagerSubsystem* TurnSys = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (!TurnSys) return;

	UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] P Key Pressed. Current IsCombatActive: %s"),
		TurnSys->IsCombatActive() ? TEXT("True") : TEXT("False"));

	// 1. 전투 중일 때 -> 전투 종료!
	if (TurnSys->IsCombatActive())
	{
		AActor* LastActiveActor = CurrentlyControlledCharacter;

		TurnSys->EndCombat();

		// 전투 종료 후 마지막 턴 유닛을 선택 상태로 남겨둠
		DeselectAllUnits();
		if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(LastActiveActor))
		{
			SelectUnit(Selectable);
		}

		UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] Ending Combat Completed."));
	}
	// 2. 비전투 상태일 때 -> 주변 유닛 감지 후 전투 시작!
	else
	{
		TurnSys->StartCombatInRange(GetActorLocation(), 2000.0f);
	}
}

void ACameraPawn::OnTurnChangedHandler(AActor* NewActiveActor, int32 CurrentRound)
{
	CurrentlyControlledCharacter = NewActiveActor;

	// 1. 기존 선택 배열 전체 해제
	DeselectAllUnits();

	// 2. 새로운 턴 액터를 CameraPawn의 SelectedUnits 배열에 등록 및 선택!
	if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(NewActiveActor))
	{
		SelectUnit(Selectable);
	}

	// 턴이 돌아온 액터 정보를 메인 HUD에 전달!
	if (MainHUDWidget)
	{
		MainHUDWidget->SetSelectedUnit(NewActiveActor);
	}

	UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] Controlled Target Updated -> %s"),
		CurrentlyControlledCharacter ? *CurrentlyControlledCharacter->GetName() : TEXT("None"));
}

void ACameraPawn::OnDebugApplyBuffPressed()
{
	/*
	if (DebugBuffRow.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[CameraPawn] DebugBuffRow is NULL! Please set it in Editor Details Panel."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] B Key Pressed -> Applying Debug Buff..."));

	// 현재 선택된 유닛들에게 버프 부여
	for (const TScriptInterface<ISelectableInterface>& Unit : SelectedUnits)
	{
		if (UObject* UnitObj = Unit.GetObject())
		{
			// IBuffableInterface 구현 여부 체크 및 호출
			if (IBuffableInterface* BuffTarget = Cast<IBuffableInterface>(UnitObj))
			{
				BuffTarget->ApplyBuffToSelf(DebugBuffRow);
			}
		}
	}
	*/
	// 현재 선택된 캐릭터에게 더미 아이템 2개 강제 주입!
	/*
	if (SelectedUnits.Num() > 0)
	{
		if (AActor* SelectedActor = Cast<AActor>(SelectedUnits[0].GetObject()))
		{
			if (IInventoryHolderInterface* InvHolder = Cast<IInventoryHolderInterface>(SelectedActor))
			{
				if (UInventoryComponent* InvComp = InvHolder->GetInventoryComponent())
				{
					//  1. 더미 포션 아이템
					FItemMasterDataRow DummyPotion;
					DummyPotion.ItemID = FName("Item_Potion_HP");
					DummyPotion.ItemName = FText::FromString(TEXT("Healing Potion"));
					DummyPotion.ItemDescription = FText::FromString(TEXT("Restores 2d4+2 HP."));
					DummyPotion.ItemType = EItemType::Consumable;
					DummyPotion.bIsStackable = true;
					DummyPotion.MaxStackCount = 99;

					// 2. 더미 검 아이템
					FItemMasterDataRow DummySword;
					DummySword.ItemID = FName("Item_Weapon_Sword");
					DummySword.ItemName = FText::FromString(TEXT("Longsword +1"));
					DummySword.ItemDescription = FText::FromString(TEXT("A well-crafted longsword."));
					DummySword.ItemType = EItemType::Weapon;
					DummySword.EquipSlot = EEquipmentSlot::MainHand;
					DummySword.bIsStackable = false;

					// 인벤토리에 아이템 추가 실행
					InvComp->AddItem(DummyPotion, 5); // 포션 5개
					InvComp->AddItem(DummySword, 1);  // 검 1개

					UE_LOG(LogTemp, Warning, TEXT("[Debug] Added Dummy Items to Selected Character's Inventory!"));
				}
			}
		}
	}
	*/
}

void ACameraPawn::OnDebugCastSkillPressed()
{
	// N키 입력 시 선택된 유닛들의 "0번 슬롯 스킬(첫 번째 학습 스킬)" 조준 명령 하달!
	for (const TScriptInterface<ISelectableInterface>& Unit : SelectedUnits)
	{
		if (ISkillCasterInterface* Caster = Cast<ISkillCasterInterface>(Unit.GetObject()))
		{
			Caster->SetPendingSkillBySlot(0); // 0번 슬롯 스킬 준비
		}
	}
}

bool ACameraPawn::IsPointerOverUI() const
{
	if (!FSlateApplication::IsInitialized()) return false;

	// 1. DPI 스케일링이 반영된 실제 시스템 마우스 절대 좌표 가져오기
	FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();

	// 2. 해당 마우스 절대 좌표 아래에 있는 Slate 위젯 경로(Widget Path) 찾기
	FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse(
		CursorPos,
		FSlateApplication::Get().GetInteractiveTopLevelWindows()
	);

	if (WidgetPath.IsValid())
	{
		// 마우스 바로 아래의 최상단 Slate 위젯
		TSharedPtr<SWidget> HoveredWidget = WidgetPath.GetLastWidget();
		if (HoveredWidget.IsValid())
		{
			FString WidgetType = HoveredWidget->GetTypeAsString();

			// 최상단 위젯이 게임 뷰포트(SViewport)나 뷰포트 레이어 매니저가 아니면 실제 UMG UI 위로 판정!
			if (WidgetType != TEXT("SViewport") &&
				WidgetType != TEXT("SGameLayerManager") &&
				WidgetType != TEXT("SConstraintCanvas"))
			{
				return true;
			}
		}
	}

	return false;
}

void ACameraPawn::StartSelectionDrag()
{
	if (IsPointerOverUI()) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	bIsDragging = true;
	PC->GetMousePosition(DragStartScreenPosition.X, DragStartScreenPosition.Y);
	DragCurrentScreenPosition = DragStartScreenPosition;
}

void ACameraPawn::EndSelectionDrag()
{
	if (!bIsDragging) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->GetMousePosition(DragCurrentScreenPosition.X, DragCurrentScreenPosition.Y);

		float DragDistance = FVector2D::Distance(DragStartScreenPosition, DragCurrentScreenPosition);

		// 일정 거리(15픽셀) 이상 움직였으면 사각형 범위 선택 실행
		if (DragDistance > 15.0f)
		{
			SelectUnitsInRectangle();
		}
		else
		{
			// 드래그 거리가 짧으면 기존 단일 클릭 선택 함수 호출
			OnSelectPressed();
		}
	}

	bIsDragging = false;
}

void ACameraPawn::SelectUnitsInRectangle()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UUnitRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UUnitRegistrySubsystem>();
	if (!Registry) return;

	const bool bIsShiftDown = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift);
	if (!bIsShiftDown)
	{
		DeselectAllUnits();
	}

	// 드래그 영역 좌표 정렬
	FVector2D MinPoint(
		FMath::Min(DragStartScreenPosition.X, DragCurrentScreenPosition.X),
		FMath::Min(DragStartScreenPosition.Y, DragCurrentScreenPosition.Y)
	);
	FVector2D MaxPoint(
		FMath::Max(DragStartScreenPosition.X, DragCurrentScreenPosition.X),
		FMath::Max(DragStartScreenPosition.Y, DragCurrentScreenPosition.Y)
	);

	int32 SelectedCount = 0;

	// 서브시스템에 등록된 유닛들만 순회 ($O(M)$ 연산)
	const TArray<TWeakObjectPtr<AActor>>& RegisteredUnits = Registry->GetSelectableUnits();

	for (int32 i = RegisteredUnits.Num() - 1; i >= 0; --i)
	{
		AActor* Actor = RegisteredUnits[i].Get();
		if (!IsValid(Actor)) continue;

		FVector2D ScreenPosition;
		// 3D 월드 좌표 -> 2D 스크린 좌표 변환 (Frustum 시야 체크 포함)
		if (PC->ProjectWorldLocationToScreen(Actor->GetActorLocation(), ScreenPosition))
		{
			// 드래그 박스 내부 체크
			if (ScreenPosition.X >= MinPoint.X && ScreenPosition.X <= MaxPoint.X &&
				ScreenPosition.Y >= MinPoint.Y && ScreenPosition.Y <= MaxPoint.Y)
			{
				if (ISelectableInterface* Selectable = Cast<ISelectableInterface>(Actor))
				{
					SelectUnit(Selectable);
					SelectedCount++;
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CameraPawn] Subsystem-Based Selected Units Count: %d"), SelectedCount);
}

void ACameraPawn::RegisterSelectableUnit(AActor* UnitToRegister)
{
	if (UnitToRegister && UnitToRegister->Implements<USelectableInterface>())
	{
		SelectableUnitRegistry.AddUnique(UnitToRegister);
		UE_LOG(LogTemp, Log, TEXT("[CameraPawn] Registered Unit: %s (Total: %d)"),
			*UnitToRegister->GetName(), SelectableUnitRegistry.Num());
	}
}

void ACameraPawn::UnregisterSelectableUnit(AActor* UnitToUnregister)
{
	if (UnitToUnregister)
	{
		SelectableUnitRegistry.Remove(UnitToUnregister);
		UE_LOG(LogTemp, Log, TEXT("[CameraPawn] Unregistered Unit: %s (Total: %d)"),
			*UnitToUnregister->GetName(), SelectableUnitRegistry.Num());
	}
}

void ACameraPawn::ToggleInventoryUI()
{
	if (!PlayerUIComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[CameraPawn] PlayerUIComponent is NULL!"));
		return;
	}

	AActor* TargetActorToOpen = nullptr;

	if (SelectedUnits.Num() > 0)
	{
		TargetActorToOpen = Cast<AActor>(SelectedUnits[0].GetObject());
	}
	else if (CurrentlyControlledCharacter)
	{
		TargetActorToOpen = CurrentlyControlledCharacter;
	}

	UE_LOG(LogTemp, Warning, TEXT("[1. CameraPawn] Opening Inventory for Target: %s"),
		TargetActorToOpen ? *TargetActorToOpen->GetName() : TEXT("NULL"));

	PlayerUIComponent->SetSelectedTargetActor(TargetActorToOpen);
	PlayerUIComponent->ToggleInventoryUI();
}