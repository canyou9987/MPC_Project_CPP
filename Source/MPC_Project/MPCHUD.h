// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MPCHUD.generated.h"


UCLASS()
class MPC_PROJECT_API AMPCHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	// 매 프레임 렌더링 타임에 안전하게 Canvas 2D 그래픽을 그리는 함수
	UFUNCTION()
	virtual void DrawHUD() override;
};
