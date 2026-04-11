// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace RPGGameplayTags
{
	//Input Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EquipSword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_UnequipSword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_LightAttack_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_HeavyAttack_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Roll);

	//Player Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Equip_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Unequip_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Attack_Light_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Attack_Heavy_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_HitPause);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Roll);
	
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Weapon_Sword);
	
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Equip_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Unequip_Sword);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_HitPause);

	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Status_JumpToFinish);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Status_Rolling);

	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_SetByCaller_AttackType_Light);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_SetByCaller_AttackType_Heavy);
	
	//Enemy Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Melee);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Ranged);
	
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Weapon);

	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Status_Strafing);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Status_UnderAttack);

	//Shared Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Ability_HitReact);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Ability_Death);
	//Shared Event Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_MeleeHit);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_HitReact);

	//Shared Combo Window Event Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_ComboWindow_Open);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_ComboWindow_Close);

	//Shared Melee Collision Event Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_Melee_CollisionEnable);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_Melee_CollisionDisable);

	//Shared SetByCaller Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_SetByCaller_BaseDamage);

	//Shared Status Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_Dead);

	//Shared HitReact Direction Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Front);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Back);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Left);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Right);
}
