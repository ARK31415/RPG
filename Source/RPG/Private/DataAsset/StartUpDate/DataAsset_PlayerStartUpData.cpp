// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/StartUpDate/DataAsset_PlayerStartUpData.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"

void UDataAsset_PlayerStartUpData::GiveToAbilitySystemComponent(URPGAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

	for(FRPGPlayerAbilitySet& AbilitySet:PlayerStartUpAbilitySet)
	{
		if(!AbilitySet.IsValid())
			continue;

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);

		InASCToGive->GiveAbility(AbilitySpec);
	}
}
