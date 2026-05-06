// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_AttackCombo.h"
#include "RPGEnemyAbility_Attack1.generated.h"

/**
 * 敌人攻击1（Enemy层）
 * TODO: 具体实现待敌人系统完善后补充
 */
UCLASS()
class RPG_API URPGEnemyAbility_Attack1 : public URPGEnemyAbility_AttackCombo
{
	GENERATED_BODY()

public:
	URPGEnemyAbility_Attack1();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
