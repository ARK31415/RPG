// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RPGMenuGameMode.generated.h"

/**
 * 主菜单 GameMode（轻量级）
 * 职责：不生成 Pawn，仅负责主菜单 UI 的初始环境
 */
UCLASS()
class RPG_API ARPGMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARPGMenuGameMode();
};
