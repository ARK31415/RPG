// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_HeavyAttack_Sword.h"

#include "RPGGameplayTags.h"

URPGPlayerAbility_HeavyAttack_Sword::URPGPlayerAbility_HeavyAttack_Sword()
{
	// GAS能力Tag配置
	AbilityTags.AddTag(RPGGameplayTags::Player_Ability_Attack_Heavy_Sword);

	// 阻止其他攻击能力（连招期间不可同时触发其他攻击）

}
