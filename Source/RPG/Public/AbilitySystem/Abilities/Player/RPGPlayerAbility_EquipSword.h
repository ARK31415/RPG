// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "RPGPlayerAbility_EquipSword.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class ARPGPlayerWeapon;
/**
 * 装备剑能力：链接武器动画层、添加WeaponInputMappingContext、赋予DefaultWeaponAbilities
 */
UCLASS()
class RPG_API URPGPlayerAbility_EquipSword : public URPGPlayerGameplayAbility
{
	GENERATED_BODY()

protected:
	// 附加到角色骨骼上的Socket名称
	UPROPERTY(EditDefaultsOnly, Category = "RPGShareAbility_SpawnWeapon")
	FName SocketNameToAttach;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	UPROPERTY()
	ARPGPlayerWeapon* CachedSwordWeapon;
	UPROPERTY()
	ARPGPlayerCharacter* CachedPlayerCharacter;
	UPROPERTY()
	ARPGPlayerController* CachedPlayerController;
	UPROPERTY()
	UPlayerCombatComponent* CombatComponent;
	
	ARPGPlayerWeapon* GetSwordWeapon() const;
	void LinkWeaponAnimLayer(ARPGPlayerWeapon* Weapon);
	void AddWeaponInputMappingContext(ARPGPlayerWeapon* Weapon);
	void GrantWeaponAbilities(ARPGPlayerWeapon* Weapon);
	
	// 蒙太奇回调
	UFUNCTION()
	void OnEquipMontageCompleted();
	UFUNCTION()
	void OnEquipMontageBlendOut();
	UFUNCTION()
	void OnEquipMontageInterrupted();
	UFUNCTION()
	void OnEquipMontageCancelled();
	
	// GameplayEvent回调
	UFUNCTION()
	void OnEquipGameplayEventReceived(FGameplayEventData Payload);
	
	// 等待装备事件
	void WaitForEquipGameplayEvent();
	
	// 附加武器到角色
	void AttachWeaponToCharacter();
};
