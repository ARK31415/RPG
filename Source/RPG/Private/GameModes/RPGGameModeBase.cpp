// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RPGGameModeBase.h"
#include "Character/RPGPlayerState.h"
#include "Controllers/RPGPlayerController.h"
#include "RPGDebugHelper.h"

DEFINE_LOG_CATEGORY(LogRPG);

ARPGGameModeBase::ARPGGameModeBase()
{
	PlayerStateClass = ARPGPlayerState::StaticClass();
	PlayerControllerClass = ARPGPlayerController::StaticClass();
}