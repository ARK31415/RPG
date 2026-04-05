// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGGameplayAbility.h"
#include "RPGPlayerGameplayAbility.generated.h"

class UPlayerCombatComponent;
class ARPGPlayerController;
class ARPGPlayerCharacter;
class UPlayerCombatComponent;

/**
 * 
 */
UCLASS()
class RPG_API URPGPlayerGameplayAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	ARPGPlayerCharacter* GetPlayerCharacterFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	ARPGPlayerController* GetPlayerControllerFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	UPlayerCombatComponent* GetPlayerCombatComponentFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	FGameplayEffectSpecHandle MakePlayerDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InWeaponBaseDamage, FGameplayTag InCurrentAttackTag, int32 InUsedComboCount);

private:
	TWeakObjectPtr<ARPGPlayerCharacter> CacheRPGPlayerCharacter;
	TWeakObjectPtr<ARPGPlayerController> CacheRPGPlayerController;
};
