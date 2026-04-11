// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGAbility_MeleeAttackCombo.h"
#include "RPGAbility_HeavyAttackCombo.generated.h"

class UPlayerCombatComponent;

/**
 * 重击连招基类（Player层）
 * 所有武器的重击连招继承此类
 * 重击特有规则：更高伤害，更长窗口期，未来不可转换到轻击
 */
UCLASS(Abstract)
class RPG_API URPGAbility_HeavyAttackCombo : public URPGAbility_MeleeAttackCombo
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 重写GetCombatComponent返回PlayerCombatComponent
	virtual UPawnCombatComponent* GetCombatComponent() const override;

	/** 获取PlayerCombatComponent的便捷方法 */
	UPlayerCombatComponent* GetPlayerCombatComponent() const;
};
