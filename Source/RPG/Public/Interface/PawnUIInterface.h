// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnUIInterface.generated.h"

class URPGPlayerUIComponent;
class URPGEnemyUIComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPawnUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Pawn UI 访问接口
 * 职责：提供统一的接口获取 UIComponent，让 Widget 不需要知道 Character 的具体类型
 */
class RPG_API IPawnUIInterface
{
	GENERATED_BODY()

public:
	/** 获取 Player UI 组件（Player 实现） */
	virtual URPGPlayerUIComponent* GetPlayerUIComponent() { return nullptr; }
	
	/** 获取 Enemy UI 组件（Enemy 实现） */
	virtual URPGEnemyUIComponent* GetEnemyUIComponent() { return nullptr; }
};
