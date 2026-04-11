// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGAbility_AttackCombo.h"
#include "RPGAbility_MeleeAttackCombo.generated.h"

class UPawnCombatComponent;

/**
 * 近战连招基类（Share层）
 * 在AttackCombo基础上增加近战特有逻辑：碰撞检测、命中事件
 * 玩家近战和敌人近战均继承此类
 */
UCLASS(Abstract)
class RPG_API URPGAbility_MeleeAttackCombo : public URPGAbility_AttackCombo
{
	GENERATED_BODY()

protected:
	// ========== Ability生命周期 ==========

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// ========== 近战特有逻辑 ==========

	/** 获取CombatComponent（子类实现：玩家返回Player版，敌人返回Enemy版） */
	virtual UPawnCombatComponent* GetCombatComponent() const;

	/** 开启武器碰撞检测（由AnimNotify触发的GameplayEvent驱动） */
	UFUNCTION()
	virtual void OnWeaponCollisionEnable(FGameplayEventData Payload);

	/** 关闭武器碰撞检测（由AnimNotify触发的GameplayEvent驱动） */
	UFUNCTION()
	virtual void OnWeaponCollisionDisable(FGameplayEventData Payload);

private:
	/** 等待武器碰撞开关事件 */
	void WaitForWeaponCollisionEvents();
};
