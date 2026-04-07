// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbilitySystemComponent, All, All)

void URPGAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid())
	{
		UE_LOG(LogRPGAbilitySystemComponent, Warning, TEXT("OnAbilityInputPressed: InputTag is invalid!"));
		return;
	}

	UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("OnAbilityInputPressed: Received InputTag [%s]"), *InputTag.ToString());

	bool bFoundMatchingAbility = false;
	for(const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("  Checking Ability [%s], DynamicTags: [%s]"),
			AbilitySpec.Ability ? *AbilitySpec.Ability->GetClass()->GetName() : TEXT("null"),
			*AbilitySpec.GetDynamicSpecSourceTags().ToStringSimple());

		if(!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) continue;

		bFoundMatchingAbility = true;
		UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("  -> Found matching Ability [%s], attempting to activate..."),
			AbilitySpec.Ability ? *AbilitySpec.Ability->GetClass()->GetName() : TEXT("null"));
				
		// 诊断信息
		if (AbilitySpec.Ability)
		{
			UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("     - Level: %d"), AbilitySpec.Level);
			UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("     - InputID: %d"), AbilitySpec.InputID);
		}
				
		const bool bSuccess = TryActivateAbility(AbilitySpec.Handle);
		UE_LOG(LogRPGAbilitySystemComponent, Log, TEXT("  -> TryActivateAbility result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	}

	if(!bFoundMatchingAbility)
	{
		UE_LOG(LogRPGAbilitySystemComponent, Warning, TEXT("OnAbilityInputPressed: No ability found with InputTag [%s]"), *InputTag.ToString());
	}
}

void URPGAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
}

void URPGAbilitySystemComponent::GrantPlayerWeaponAbility(const TArray<FRPGPlayerAbilitySet>& InDefaultWeaponAbility, int32 ApplyLevel, TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if(InDefaultWeaponAbility.IsEmpty())
	{
		return;
	}

	for(const FRPGPlayerAbilitySet& AbilitySet : InDefaultWeaponAbility)
	{
		if(!AbilitySet.IsValid()) continue;

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);

		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}
}

void URPGAbilitySystemComponent::RemovedGrantPlayerWeaponAbility(TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove)
{
	if(InSpecHandlesToRemove.IsEmpty())
	{
		return;
	}

	for(const FGameplayAbilitySpecHandle& SpecHandle: InSpecHandlesToRemove)
	{
		if(SpecHandle.IsValid())
		{
			ClearAbility(SpecHandle);
		}
	}

	InSpecHandlesToRemove.Empty();
}

bool URPGAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate)
{
	check(AbilityTagToActivate.IsValid());

	TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTagToActivate.GetSingleTagContainer(), FoundAbilitySpecs);

	if(!FoundAbilitySpecs.IsEmpty())
	{
		const int32 RandomAbilityIndex = FMath::RandRange(0, FoundAbilitySpecs.Num()-1);
		FGameplayAbilitySpec* SpecToActive = FoundAbilitySpecs[RandomAbilityIndex];

		check(SpecToActive);

		if(!SpecToActive->IsActive())
		{
			return TryActivateAbility(SpecToActive->Handle);
		}
	}
	return false;
}
