// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnDeathInterface.generated.h"

/**
 * Pawn Death Interface
 * 职责：定义死亡事件的接口，由 Character 实现
 * 
 * Ability 在检测到死亡时调用此接口通知 Character
 * Character 应在 OnDeathStarted 中处理：
 *   - 禁用碰撞、停止移动
 *   - 暂停动画（bPauseAnims）
 *   - 停止 AI 行为树
 *   - 禁用战斗组件
 * 
 * Character 应在 OnDeathFinished 中处理：
 *   - 对象池回收（ReleaseEnemy）
 *   - 或直接销毁（SetLifeSpan）
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
