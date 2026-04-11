// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_AttackCombo.h"
#include "RPGPlayerAbility_LightAttack.generated.h"

/**
 * 轻击连招基类（Player层）
 * 所有武器的轻击连招继承此类
 * 轻击特有规则：较快的窗口期，未来可支持转换到重击
 */
UCLASS(Abstract)
class RPG_API URPGPlayerAbility_LightAttack : public URPGPlayerAbility_AttackCombo
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
