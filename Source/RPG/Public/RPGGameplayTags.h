// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace RPGGameplayTags
{
	//Input Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look);

	//Player Event Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_HitPause);

	//Shared Event Tags
	RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_MeleeHit);

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
