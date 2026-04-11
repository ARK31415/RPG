// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_Melee2.h"
#include "RPGGameplayTags.h"

URPGEnemyAbility_Melee2::URPGEnemyAbility_Melee2()
{
	AbilityTags.AddTag(RPGGameplayTags::Enemy_Ability_Melee);
}

void URPGEnemyAbility_Melee2::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TODO: 实现敌人近战攻击2逻辑
	// 注意：父类URPGAbility_EnemyAttackCombo已经处理了Montage播放和连招逻辑
}
