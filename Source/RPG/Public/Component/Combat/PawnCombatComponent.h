// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Component/PawnExtensionComponentBase.h"
#include "PawnCombatComponent.generated.h"

class ARPGWeaponBase;

UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	CurrentEquippedWeapon,
	LeftHand,
	RightHand
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RPG|Weapon")
	void RegisterSpawnWeapon(FGameplayTag InWeaponTagToRegister, ARPGWeaponBase* InWeaponToRegister, bool bRegisterAsEquippedWeapon = false);

	UFUNCTION(BlueprintCallable, Category="RPG|Weapon")
	ARPGWeaponBase* GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;

	UPROPERTY(BlueprintReadWrite, Category="RPG|Weapon")
	FGameplayTag CurrentEquippedWeaponTag;

	UFUNCTION(BlueprintCallable, Category="RPG|Weapon")
	ARPGWeaponBase* GetCharacterCurrentEquippedWeapon()const;

	UFUNCTION(BlueprintCallable, Category="RPG|Weapon")
	void ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType = EToggleDamageType::CurrentEquippedWeapon);

	virtual void OnHitTargetActor(AActor* HitActor);
	virtual void OnWeaponPullerFromTargetActor(AActor* InteractedActor);

protected:
	UPROPERTY()
	TArray<AActor*> OverlappedActors;

private:
	TMap<FGameplayTag, ARPGWeaponBase*> CharacterCarriedWeaponMap;
};
