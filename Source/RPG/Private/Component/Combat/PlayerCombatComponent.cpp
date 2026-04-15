// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/PlayerCombatComponent.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerCombatComponent, All, All)

UPlayerCombatComponent::UPlayerCombatComponent()
{
	CurrentComboCount = 1;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentComboCount = 1;
}

void UPlayerCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UPlayerCombatComponent::ResetComboCount()
{
	CurrentComboCount = 1;
	UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Combo count reset to 1 (timer expired)"));
}

void UPlayerCombatComponent::AdvanceComboCount(int32 MaxComboCount)
{
	if (CurrentComboCount >= MaxComboCount)
	{
		CurrentComboCount = 1;
		UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Max combo reached, resetting to 1"));
	}
	else
	{
		CurrentComboCount++;
		UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Combo count advanced to %d"), CurrentComboCount);
	}
}

void UPlayerCombatComponent::OnComboWindowTimerExpired()
{
	UE_LOG(LogRPGPlayerCombatComponent, Warning, TEXT("[PlayerCombatComponent] >>> Combo window timer expired, resetting combo <<<"));
	ResetComboCount();
}

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
