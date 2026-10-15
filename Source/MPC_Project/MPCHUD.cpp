// Fill out your copyright notice in the Description page of Project Settings.


#include "MPCHUD.h"
#include "CameraPawn.h"

void AMPCHUD::DrawHUD()
{
	Super::DrawHUD();

	ACameraPawn* Pawn = Cast<ACameraPawn>(GetOwningPawn());
	if (!Pawn) return;

	// CameraPawn이 드래그 중일 때만 안전하게 렌더링
	if (Pawn->IsDragging())
	{
		FVector2D StartPos = Pawn->GetDragStartPos();
		FVector2D CurrentPos = Pawn->GetDragCurrentPos();

		float Width = CurrentPos.X - StartPos.X;
		float Height = CurrentPos.Y - StartPos.Y;

		// 반투명 초록색 사각형 채우기
		DrawRect(FLinearColor(0.0f, 1.0f, 0.0f, 0.15f), StartPos.X, StartPos.Y, Width, Height);

		// 외곽선 그리기
		DrawLine(StartPos.X, StartPos.Y, CurrentPos.X, StartPos.Y, FLinearColor::Green, 1.5f);
		DrawLine(CurrentPos.X, StartPos.Y, CurrentPos.X, CurrentPos.Y, FLinearColor::Green, 1.5f);
		DrawLine(CurrentPos.X, CurrentPos.Y, StartPos.X, CurrentPos.Y, FLinearColor::Green, 1.5f);
		DrawLine(StartPos.X, CurrentPos.Y, StartPos.X, StartPos.Y, FLinearColor::Green, 1.5f);
	}
}
