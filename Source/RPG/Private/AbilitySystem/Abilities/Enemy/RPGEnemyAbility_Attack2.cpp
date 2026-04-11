// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_Attack2.h"

#include "RPGGameplayTags.h"

URPGEnemyAbility_Attack2::URPGEnemyAbility_Attack2()
{
	AbilityTags.AddTag(RPGGameplayTags::Enemy_Ability_Ranged);
}

void URPGEnemyAbility_Attack2::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TODO: 实现敌人攻击2逻辑
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void URPGEnemyAbility_Attack2::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
