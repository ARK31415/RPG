// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_Melee1.h"

#include "RPGGameplayTags.h"

URPGEnemyAbility_Melee1::URPGEnemyAbility_Melee1()
{
	AbilityTags.AddTag(RPGGameplayTags::Enemy_Ability_Melee);
}

void URPGEnemyAbility_Melee1::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TODO: 实现敌人近战攻击1逻辑
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void URPGEnemyAbility_Melee1::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
