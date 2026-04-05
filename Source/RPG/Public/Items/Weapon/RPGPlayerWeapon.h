// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "RPGWeaponBase.h"
#include "Types/RPGStructTypes.h"
#include "RPGPlayerWeapon.generated.h"


UCLASS()
class RPG_API ARPGPlayerWeapon : public ARPGWeaponBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponData")
	FRPGPlayerWeaponData PlayerWeaponData;

	UFUNCTION(BlueprintCallable)
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles);

	UFUNCTION(BlueprintPure)
	TArray<FGameplayAbilitySpecHandle> GetGrantAbilitySpecHandles() const;
	
private:
	TArray<FGameplayAbilitySpecHandle> GrantAbilitySpecHandles;
};
