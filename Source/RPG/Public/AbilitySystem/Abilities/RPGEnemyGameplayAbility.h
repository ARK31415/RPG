// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGEnemyGameplayAbility.generated.h"

class ARPGEnemyCharacter;
class UEnemyCombatComponent;
class UGameplayEffect;

/**
 * 敌人Ability基类
 * 提供敌人特有的辅助方法：AI控制器访问、EnemyCombatComponent获取等
 * 所有敌人Ability均继承此类
 */
UCLASS(Abstract)
class RPG_API URPGEnemyGameplayAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	/** 获取敌人角色 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	ARPGEnemyCharacter* GetEnemyCharacterFromActorInfo() const;

	/** 获取敌人战斗组件 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo() const;

	/** 创建敌人伤害 GE Spec（从 AttackPower 属性获取基础伤害） */
	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	FGameplayEffectSpecHandle MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InBaseDamage);
};
