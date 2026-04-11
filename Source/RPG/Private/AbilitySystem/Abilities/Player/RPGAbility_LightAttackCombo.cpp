// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGAbility_LightAttackCombo.h"

#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Character/RPGPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbility_LightAttackCombo, All, All)

void URPGAbility_LightAttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGAbility_LightAttackCombo, Log, TEXT("[%s] ActivateAbility - LightAttackCombo"), *GetName());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UPawnCombatComponent* URPGAbility_LightAttackCombo::GetCombatComponent() const
{
	return GetPlayerCombatComponent();
}

UPlayerCombatComponent* URPGAbility_LightAttackCombo::GetPlayerCombatComponent() const
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(AvatarActor))
		{
			return PlayerCharacter->GetPlayerCombatComponent();
		}
	}
	return nullptr;
}
