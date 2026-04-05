// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/RPGPlayerWeapon.h"


void ARPGPlayerWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles)
{
	GrantAbilitySpecHandles = InSpecHandles;
}

TArray<FGameplayAbilitySpecHandle> ARPGPlayerWeapon::GetGrantAbilitySpecHandles() const
{
	return GrantAbilitySpecHandles;
}

