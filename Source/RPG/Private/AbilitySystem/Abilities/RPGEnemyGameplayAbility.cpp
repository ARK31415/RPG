// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/RPGEnemyGameplayAbility.h"

#include "Character/RPGEnemyCharacter.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"

ARPGEnemyCharacter* URPGEnemyGameplayAbility::GetEnemyCharacterFromActorInfo() const
{
	return Cast<ARPGEnemyCharacter>(GetAvatarActorFromActorInfo());
}

UEnemyCombatComponent* URPGEnemyGameplayAbility::GetEnemyCombatComponentFromActorInfo() const
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		return AvatarActor->FindComponentByClass<UEnemyCombatComponent>();
	}
	return nullptr;
}

FGameplayEffectSpecHandle URPGEnemyGameplayAbility::MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InBaseDamage)
{
	URPGAbilitySystemComponent* OwningASC = GetRPGAbilitySystemComponentFromActorInfo();
	if (!OwningASC || !EffectClass)
	{
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectContextHandle ContextHandle = OwningASC->MakeEffectContext();
	ContextHandle.SetAbility(this);
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle EffectSpecHandle = OwningASC->MakeOutgoingSpec(
		EffectClass,
		GetAbilityLevel(),
		ContextHandle
	);

	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		RPGGameplayTags::Shared_SetByCaller_BaseDamage,
		InBaseDamage
	);

	return EffectSpecHandle;
}
