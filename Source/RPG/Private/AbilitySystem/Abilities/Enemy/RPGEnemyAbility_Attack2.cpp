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
	// 注意：父类URPGAbility_EnemyAttackCombo已经处理了Montage播放和连招逻辑
}
