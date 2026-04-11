// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_HeavyAttack_Sword.h"

#include "RPGGameplayTags.h"

URPGPlayerAbility_HeavyAttack_Sword::URPGPlayerAbility_HeavyAttack_Sword()
{
	// 连招配置（Montage在蓝图子类中通过CDO设置）
	MaxComboCount = 2;
	ComboWindowTime = 0.6f;

	// GAS能力Tag配置
	AbilityTags.AddTag(RPGGameplayTags::Player_Ability_Attack_Heavy_Sword);

	// 阻止其他攻击能力（连招期间不可同时触发其他攻击）
	BlockAbilitiesWithTag.AddTag(RPGGameplayTags::Player_Ability_Attack_Light_Sword);
	BlockAbilitiesWithTag.AddTag(RPGGameplayTags::Player_Ability_Attack_Heavy_Sword);
	BlockAbilitiesWithTag.AddTag(RPGGameplayTags::Player_Ability_Roll);
}
