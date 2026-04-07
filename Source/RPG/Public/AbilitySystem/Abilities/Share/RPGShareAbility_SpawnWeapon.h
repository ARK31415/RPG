// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGShareAbility_SpawnWeapon.generated.h"

class ARPGWeaponBase;

/**
 * 通用武器生成技能：生成武器Actor -> 附加到骨骼Socket -> 登记到CombatComponent
 * ActivationPolicy 建议设为 OnGive，角色初始化时自动执行
 */
UCLASS()
class RPG_API URPGShareAbility_SpawnWeapon : public URPGGameplayAbility
{
	GENERATED_BODY()

public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 要生成的武器类
	UPROPERTY(EditDefaultsOnly, Category = "RPGShareAbility_SpawnWeapon")
	TSubclassOf<ARPGWeaponBase> WeaponClassToSpawn;
	
	// 附加到角色骨骼上的Socket名称
	UPROPERTY(EditDefaultsOnly, Category = "RPGShareAbility_SpawnWeapon")
	FName SocketNameToAttach;
	
	// 在CombatComponent中登记时使用的GameplayTag
	UPROPERTY(EditDefaultsOnly, Category = "RPGShareAbility_SpawnWeapon")
	FGameplayTag WeaponTagToRegister;

	// 是否登记为当前装备武器
	UPROPERTY(EditDefaultsOnly, Category = "RPGShareAbility_SpawnWeapon")
	bool bRegisterAsEquippedWeapon;

private:
	// 已生成的武器实例（用于EndAbility时清理）
	UPROPERTY()
	TObjectPtr<ARPGWeaponBase> SpawnedWeapon;
};
