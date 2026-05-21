// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RPGGameModeBase.h"
#include "Character/RPGPlayerState.h"
#include "RPGDebugHelper.h"

// 定义全局 RPG 日志分类
DEFINE_LOG_CATEGORY(LogRPG);

ARPGGameModeBase::ARPGGameModeBase()
{
	// Set default player state class for RPG game mode
	PlayerStateClass = ARPGPlayerState::StaticClass();
	// Note: UI is managed through UPrimaryGameLayout widget via CommonGame plugin's GameUIPolicy
	// No need to set DefaultHUDClass in this architecture
}