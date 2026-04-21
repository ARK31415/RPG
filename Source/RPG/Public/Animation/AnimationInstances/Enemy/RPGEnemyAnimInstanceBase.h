// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimationInstances/RPGBaseAnimInstance.h"
#include "Types/RPGEnumTypes.h"
#include "RPGEnemyAnimInstanceBase.generated.h"

/**
 * 敌人动画实例基类
 * 提供敌人动画所需的移动状态和战斗状态（受击、死亡、攻击）
 */
UCLASS()
class RPG_API URPGEnemyAnimInstanceBase : public URPGBaseAnimInstance
{
	GENERATED_BODY()

public:
	URPGEnemyAnimInstanceBase();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ========== 移动状态（简化版：Idle / Moving） ==========
	
	/** 是否待机（速度低于阈值） */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Movement")
	bool bIsIdle;

	/** 移动速度（0-1 归一化，用于 Blend Space） */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Movement")
	float MovementSpeed;

	/** 待机/移动速度阈值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Enemy Movement Settings")
	float IdleMoveSpeedThreshold;

	// ========== 战斗状态 ==========
	
	/** 是否正在受击反应 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Combat")
	bool bIsHitReacting;

	/** 受击方向 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Combat")
	EEnemyHitReactDirection HitReactDirection;

	/** 是否死亡 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Combat")
	bool bIsDead;

	/** 是否正在攻击 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Enemy Combat")
	bool bIsAttacking;

private:
	void UpdateMovementState();
	void UpdateCombatState();
};
