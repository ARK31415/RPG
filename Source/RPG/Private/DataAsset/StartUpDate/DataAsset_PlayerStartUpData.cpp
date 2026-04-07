// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/StartUpDate/DataAsset_PlayerStartUpData.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGDataAsset_PlayerStartUpData, All, All)

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

		UE_LOG(LogRPGDataAsset_PlayerStartUpData, Log, TEXT("GiveToASC: Granted Ability [%s] with InputTag [%s] at Level [%d]"),
			*AbilitySet.AbilityToGrant->GetName(),
			*AbilitySet.InputTag.ToString(),
			ApplyLevel);
	}
}
