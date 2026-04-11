// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/RPGAbility_EnemyAttackCombo.h"
#include "RPGEnemyAbility_Attack2.generated.h"

/**
 * 敌人攻击2（Enemy层）
 * TODO: 具体实现待敌人系统完善后补充
 */
UCLASS()
class RPG_API URPGEnemyAbility_Attack2 : public URPGAbility_EnemyAttackCombo
{
	GENERATED_BODY()

public:
	URPGEnemyAbility_Attack2();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
