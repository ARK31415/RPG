// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Enemy/Grunting/RPGEnemyAbility_Grunting_Guardian_SpawnWeapon.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGGameplayTags.h"

URPGEnemyAbility_Grunting_Guardian_SpawnWeapon::URPGEnemyAbility_Grunting_Guardian_SpawnWeapon()
{
	// 设置WeaponTagToRegister为Enemy.Weapon
	WeaponTagToRegister = RPGGameplayTags::Enemy_Weapon;
	
	// 设置Ability Activation Policy为OnGive
	AbilityActivationPolicy = ERPGAbilityActivationPolicy::OnGive;
}
