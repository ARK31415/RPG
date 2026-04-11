// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/RPGEnemyGameplayAbility.h"
#include "RPGEnemyAbility_Melee1.generated.h"

/**
 * 敌人近战攻击1（Enemy层）
 * TODO: 具体实现待敌人系统完善后补充
 */
UCLASS()
class RPG_API URPGEnemyAbility_Melee1 : public URPGEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	URPGEnemyAbility_Melee1();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
