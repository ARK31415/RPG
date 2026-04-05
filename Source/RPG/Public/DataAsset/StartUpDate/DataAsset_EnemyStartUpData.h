// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/StartUpDate/DataAsset_StartUpDataBase.h"
#include "DataAsset_EnemyStartUpData.generated.h"

/**
 * Enemy Startup Data - 用于敌人的初始能力和效果配置
 */
UCLASS()
class RPG_API UDataAsset_EnemyStartUpData : public UDataAsset_StartUpDataBase
{
	GENERATED_BODY()

public:
	// Can add enemy-specific startup logic here if needed
	// For now, it inherits all functionality from base class
};
