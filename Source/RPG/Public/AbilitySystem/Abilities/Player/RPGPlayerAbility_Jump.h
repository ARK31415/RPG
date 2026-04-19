// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "RPGPlayerAbility_Jump.generated.h"

class UAbilityTask_WaitGameplayEvent;

/**
 * 玩家跳跃Ability
 * 职责：跳跃判定、跳跃状态Tag管理、执行跳跃
 * 
 * 设计原则：
 * - Character管理土狼时间状态，Ability查询Character::CanJump()进行判定
 * - 使用Player_Status_Jumping Tag标记跳跃状态，供其他系统查询
 * - Ability生命周期与跳跃动画同步，通过Event.Jump.Finished事件结束
 */
UCLASS()
class RPG_API URPGPlayerAbility_Jump : public URPGPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	URPGPlayerAbility_Jump();

protected:
	// ========== Ability生命周期 ==========
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
								 const FGameplayAbilityActorInfo* ActorInfo, 
								 const FGameplayAbilityActivationInfo ActivationInfo, 
								 const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
						   const FGameplayAbilityActorInfo* ActorInfo, 
						   const FGameplayAbilityActivationInfo ActivationInfo, 
						   bool bReplicateEndAbility, 
						   bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, 
									const FGameplayAbilityActorInfo* ActorInfo, 
									const FGameplayTagContainer* SourceTags = nullptr, 
									const FGameplayTagContainer* TargetTags = nullptr, 
									FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// ========== Event回调 ==========
	
	/** 接收跳跃完成事件（AN_JumpFinish触发） */
	UFUNCTION()
	void OnJumpFinishedEventReceived(FGameplayEventData EventData);

private:
	// ========== 核心逻辑 ==========
	
	/** 检查是否可以跳跃（委托给Character::CanJump） */
	bool CanJump() const;

	/** 执行跳跃 */
	void PerformJump();

	/** 应用跳跃状态Tag */
	void ApplyJumpingTag();

	/** 移除跳跃状态Tag */
	void RemoveJumpingTag();

	/** 获取玩家角色 */
	ARPGPlayerCharacter* GetPlayerCharacter() const;

	/** 设置跳跃完成事件监听 */
	void SetupJumpFinishedEventWait();

	// ========== 缓存Task ==========
	
	/** 等待跳跃完成事件的Task */
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* JumpFinishedEventTask;
};
