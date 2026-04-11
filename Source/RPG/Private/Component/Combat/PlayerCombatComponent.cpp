// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/PlayerCombatComponent.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"

ARPGPlayerWeapon* UPlayerCombatComponent::GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
	return Cast<ARPGPlayerWeapon>(GetCharacterCarriedWeaponByTag(InWeaponTag)); 
}

ARPGPlayerWeapon* UPlayerCombatComponent::GetPlayerCurrentEquippedWeapon() const
{
	return Cast<ARPGPlayerWeapon>(GetCharacterCurrentEquippedWeapon());
}

float UPlayerCombatComponent::GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLeveL) const
{
	if (ARPGPlayerWeapon* Weapon = GetPlayerCurrentEquippedWeapon())
	{
		return Weapon->PlayerWeaponData.WeaponBaseDamage.GetValueAtLevel(InLeveL);
	}
	return 0.f;
}

void UPlayerCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	if(OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);

	FGameplayEventData Data;
	Data.Instigator = GetOwningPawn();
	Data.Target = HitActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Shared_Event_MeleeHit,
		Data
	);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Player_Event_HitPause,
		FGameplayEventData()
	);
}

void UPlayerCombatComponent::OnWeaponPullerFromTargetActor(AActor* InteractedActor)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Player_Event_HitPause,
		FGameplayEventData()
	);
}
