// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/RPGPlayerController.h"
#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "Character/RPGPlayerCharacter.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerInput.h"
#include "Components/InputComponent.h"
#include "RPGDebugHelper.h"

ARPGPlayerController::ARPGPlayerController()
{
	PlayerTeamId = FGenericTeamId(0);
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ARPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	if (PlayerInput && !InputComponent)
	{
		SetupInputComponent();
	}

	if (InputComponent && InPawn)
	{
		ARPGPlayerCharacter* RPGPawn = Cast<ARPGPlayerCharacter>(InPawn);
		if (RPGPawn)
		{
			RPGPawn->InitializePlayerInput(InputComponent);
		}
	}

	GetWorldTimerManager().SetTimer(InputModeTimerHandle, this, &ARPGPlayerController::EnsureGameInputMode, 0.1f, false);
}

void ARPGPlayerController::EnsureGameInputMode()
{
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	return PlayerTeamId;
}
