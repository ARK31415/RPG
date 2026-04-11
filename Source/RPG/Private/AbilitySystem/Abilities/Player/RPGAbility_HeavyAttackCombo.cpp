// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGAbility_HeavyAttackCombo.h"

#include "Component/Combat/PlayerCombatComponent.h"
#include "Character/RPGPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbility_HeavyAttackCombo, All, All)

void URPGAbility_HeavyAttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGAbility_HeavyAttackCombo, Log, TEXT("[%s] ActivateAbility - HeavyAttackCombo"), *GetName());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UPawnCombatComponent* URPGAbility_HeavyAttackCombo::GetCombatComponent() const
{
	return GetPlayerCombatComponent();
}

UPlayerCombatComponent* URPGAbility_HeavyAttackCombo::GetPlayerCombatComponent() const
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
