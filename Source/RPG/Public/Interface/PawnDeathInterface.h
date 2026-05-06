// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnDeathInterface.generated.h"

/**
 * Pawn Death Interface
 * 职责：定义死亡事件的接口，由 Character 实现
 * HealthComponent 在检测到死亡时调用此接口通知 Owner
 */
UINTERFACE(MinimalAPI)
class UPawnDeathInterface : public UInterface
{
	GENERATED_BODY()
};

class RPG_API IPawnDeathInterface
{
	GENERATED_BODY()

public:
	/**
	 * 死亡开始回调
	 * 由 HealthComponent 在检测到 CurrentHealth <= 0 时调用
	 * Character 应在此处理：设置死亡Tag、禁用AI/战斗组件、停止移动、播放死亡动画等
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Death")
	void OnDeathStarted();

	/**
	 * 死亡完成回调
	 * 由 HealthComponent 在延迟后调用（默认2秒）
	 * Character 应在此处理：销毁 Actor、掉落物品、给予经验等清理工作
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Death")
	void OnDeathFinished();
};
