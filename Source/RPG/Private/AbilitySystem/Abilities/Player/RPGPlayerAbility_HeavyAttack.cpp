// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_HeavyAttack.h"
#include "Character/RPGPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbility_HeavyAttackCombo, All, All)

void URPGPlayerAbility_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGAbility_HeavyAttackCombo, Log, TEXT("[%s] ActivateAbility - HeavyAttackCombo"), *GetName());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
