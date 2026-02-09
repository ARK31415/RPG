// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "RPGEnhancedInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserObject, typename CallbackFunc>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag,
							   ETriggerEvent TriggerEvent, UserObject* ContextObject,
							   CallbackFunc Func);

	template <class UserObject, typename CallbackFunc>
	void BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* ContextObject,
								CallbackFunc InputPressedFuc, CallbackFunc InputReleasedFuc);
};

template <class UserObject, typename CallbackFunc>
void URPGEnhancedInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag,
												   ETriggerEvent TriggerEvent,
												   UserObject* ContextObject, CallbackFunc Func)
{
	checkf(InInputConfig, TEXT("Input Config Data Asset is null"));

	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		BindAction(FoundAction, TriggerEvent, ContextObject, Func);
	}
}

template <class UserObject, typename CallbackFunc>
void URPGEnhancedInputComponent::BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig,
													UserObject* ContextObject, CallbackFunc InputPressedFuc,
													CallbackFunc InputReleasedFuc)
{
	checkf(InInputConfig, TEXT("Input Config Data Asset is null"));

	for (const FRPGInputActionConfig& AbilityInputActionConfig : InInputConfig->AbilityInputActions)
	{
		if (!AbilityInputActionConfig.IsValid())
			continue;

		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Started, ContextObject, InputPressedFuc,
				   AbilityInputActionConfig.InputTag);
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Completed, ContextObject, InputReleasedFuc,
				   AbilityInputActionConfig.InputTag);
	}
}