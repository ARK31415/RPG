// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/RPGPlayerController.h"
#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "Engine/World.h"

ARPGPlayerController::ARPGPlayerController()
{
	PlayerTeamId = FGenericTeamId(0);
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ✅ 调用 UI 管理器显示 HUD（通过 CommonUI 四层栈的 Game 层）
	if (URPGUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<URPGUIManagerSubsystem>())
	{
		//UIManager->ShowHUD(this);
		UE_LOG(LogTemp, Log, TEXT("RPGPlayerController::BeginPlay - ShowHUD called"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RPGPlayerController::BeginPlay - URPGUIManagerSubsystem not found"));
	}
}

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	return PlayerTeamId;
}
