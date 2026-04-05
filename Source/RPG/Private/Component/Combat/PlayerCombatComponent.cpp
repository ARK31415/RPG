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

// ========== 连招系统 ==========

void UPlayerCombatComponent::StartAttack()
{
	if (!CanAttack())
	{
		return;
	}

	ComboIndex = 0;
	CombatState = ERPGCombatState::Attacking;
}

void UPlayerCombatComponent::TryComboAttack()
{
	if (!bIsInComboWindow || ComboIndex >= MaxComboCount - 1)
	{
		return;
	}

	ComboIndex++;
	bIsInComboWindow = false;
}

void UPlayerCombatComponent::ResetCombo()
{
	ComboIndex = 0;
	bIsInComboWindow = false;
	CombatState = ERPGCombatState::Idle;
}

void UPlayerCombatComponent::OpenComboWindow()
{
	bIsInComboWindow = true;
}

void UPlayerCombatComponent::CloseComboWindow()
{
	bIsInComboWindow = false;

	if (CombatState == ERPGCombatState::Attacking)
	{
		ResetCombo();
	}
}

bool UPlayerCombatComponent::CanAttack() const
{
	return CombatState == ERPGCombatState::Idle || CombatState == ERPGCombatState::Combat;
}

bool UPlayerCombatComponent::IsAttacking() const
{
	return CombatState == ERPGCombatState::Attacking;
}

void UPlayerCombatComponent::SetMaxComboCount(int32 InMaxComboCount)
{
	MaxComboCount = FMath::Max(1, InMaxComboCount);
}

void UPlayerCombatComponent::SetAttackSpeedMultiplier(float NewMultiplier)
{
	AttackSpeedMultiplier = FMath::Max(0.1f, NewMultiplier);
}

void UPlayerCombatComponent::ApplyWeaponData(const FRPGPlayerWeaponData& WeaponData)
{
	SetMaxComboCount(WeaponData.MaxComboCount);
	SetAttackSpeedMultiplier(WeaponData.AttackSpeedMultiplier);
}
