// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "RPGPlayerAbility_UnequipSword.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class ARPGPlayerWeapon;
class ARPGPlayerCharacter;
class ARPGPlayerController;
class UPlayerCombatComponent;

/**
 * 卸下剑能力：取消链接武器动画层、移除WeaponInputMappingContext、移除DefaultWeaponAbilities
 */
UCLASS()
class RPG_API URPGPlayerAbility_UnequipSword : public URPGPlayerGameplayAbility
{
	GENERATED_BODY()

protected:
	// 附加到角色骨骼上的Socket名称
	UPROPERTY(EditDefaultsOnly, Category = "RPGPlayerAbility_EquipWeapon")
	FName SocketNameToAttach;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	UPROPERTY()
	ARPGPlayerWeapon* CachedEquippedWeapon;
	UPROPERTY()
	ARPGPlayerCharacter* CachedPlayerCharacter;
	UPROPERTY()
	ARPGPlayerController* CachedPlayerController;
	UPROPERTY()
	UPlayerCombatComponent* CombatComponent;

	ARPGPlayerWeapon* GetCurrentEquippedWeapon() const;
	void UnlinkWeaponAnimLayer();
	void RemoveWeaponInputMappingContext(ARPGPlayerWeapon* Weapon);
	void RemoveWeaponAbilities(ARPGPlayerWeapon* Weapon);

	// 蒙太奇回调
	UFUNCTION()
	void OnUnequipMontageCompleted();
	UFUNCTION()
	void OnUnequipMontageBlendOut();
	UFUNCTION()
	void OnUnequipMontageInterrupted();
	UFUNCTION()
	void OnUnequipMontageCancelled();

	// GameplayEvent回调
	UFUNCTION()
	void OnUnequipGameplayEventReceived(FGameplayEventData Payload);

	// 等待卸下事件
	void WaitForUnequipGameplayEvent();

	// 从角色上分离武器并执行卸下逻辑
	void DetachWeaponFromCharacter();
};
