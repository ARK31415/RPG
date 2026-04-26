// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Character/RPGPlayerCharacter.h"
#include "Controllers/RPGPlayerController.h"
#include "RPGGameplayTags.h"


ARPGPlayerCharacter* URPGPlayerGameplayAbility::GetPlayerCharacterFromActorInfo()
{
	if(!CacheRPGPlayerCharacter.IsValid())
	{
		CacheRPGPlayerCharacter = Cast<ARPGPlayerCharacter>(CurrentActorInfo->AvatarActor);
	}
	return CacheRPGPlayerCharacter.IsValid()? CacheRPGPlayerCharacter.Get():nullptr;
}

ARPGPlayerController* URPGPlayerGameplayAbility::GetPlayerControllerFromActorInfo()
{
	if(!CacheRPGPlayerController.IsValid())
	{
		CacheRPGPlayerController = Cast<ARPGPlayerController>(CurrentActorInfo->PlayerController);
	}
	return  CacheRPGPlayerController.IsValid()? CacheRPGPlayerController.Get():nullptr;
}

UPlayerCombatComponent* URPGPlayerGameplayAbility::GetPlayerCombatComponentFromActorInfo()
{
	return GetPlayerCharacterFromActorInfo()->GetPlayerCombatComponent();
}

FGameplayEffectSpecHandle URPGPlayerGameplayAbility::MakePlayerDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InWeaponBaseDamage,
	FGameplayTag InCurrentAttackTag, int32 InUsedComboCount)
{
	FGameplayEffectContextHandle ContextHandle = GetRPGAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	ContextHandle.SetAbility(this);
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle EffectSpecHandle =  GetRPGAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		EffectClass,
		GetAbilityLevel(),
		ContextHandle
	);

	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		RPGGameplayTags::Shared_SetByCaller_BaseDamage,
		InWeaponBaseDamage
	);

	if(InCurrentAttackTag.IsValid())
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTag, InUsedComboCount);
	}
	
	return EffectSpecHandle;
}
