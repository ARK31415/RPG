// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RPGGameModeBase.h"
#include "Character/RPGPlayerState.h"

ARPGGameModeBase::ARPGGameModeBase()
{
	// Set default player state class for RPG game mode
	PlayerStateClass = ARPGPlayerState::StaticClass();
}