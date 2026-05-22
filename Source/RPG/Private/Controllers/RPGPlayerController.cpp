// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/RPGPlayerController.h"
#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "RPGDebugHelper.h"

ARPGPlayerController::ARPGPlayerController()
{
	PlayerTeamId = FGenericTeamId(0);
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Debug::Log(FString::Printf(TEXT("[PlayerController] BeginPlay - Name=%s"), *GetName()));
	Debug::Log(FString::Printf(TEXT("[PlayerController] BeginPlay - Pawn=%s"), GetPawn() ? *GetPawn()->GetName() : TEXT("NULL")));

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		URPGUIManagerSubsystem* UIManager = GI->GetSubsystem<URPGUIManagerSubsystem>();
		Debug::Log(FString::Printf(TEXT("[PlayerController] BeginPlay - UIManager=%s"), UIManager ? TEXT("Found") : TEXT("NULL")));
	}
}

void ARPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Debug::Log(FString::Printf(TEXT("[PlayerController] OnPossess - Pawn=%s"), InPawn ? *InPawn->GetName() : TEXT("NULL")));

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	Debug::Log(TEXT("[PlayerController] OnPossess - InputMode set to GameOnly"));
}

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	return PlayerTeamId;
}
