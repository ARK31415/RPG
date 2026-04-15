// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_LightAttack_Sword.h"

#include "RPGGameplayTags.h"

URPGPlayerAbility_LightAttack_Sword::URPGPlayerAbility_LightAttack_Sword()
{

	// GAS能力Tag配置
	AbilityTags.AddTag(RPGGameplayTags::Player_Ability_Attack_Light_Sword);

	// 阻止其他攻击能力（连招期间不可同时触发其他攻击）

}
