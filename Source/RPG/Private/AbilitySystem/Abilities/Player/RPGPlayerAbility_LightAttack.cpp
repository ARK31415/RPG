// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_LightAttack.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "Character/RPGPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerAbility_LightAttack, All, All)

void URPGPlayerAbility_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGPlayerAbility_LightAttack, Log, TEXT("[%s] ActivateAbility - LightAttackCombo"), *GetName());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
