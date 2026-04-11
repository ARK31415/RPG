// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/RPGEnemyGameplayAbility.h"

#include "Character/RPGEnemyCharacter.h"
#include "Component/Combat/EnemyCombatComponent.h"

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
