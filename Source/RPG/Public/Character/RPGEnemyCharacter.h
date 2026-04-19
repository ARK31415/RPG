// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "RPGEnemyCharacter.generated.h"

class URPGAbilitySystemComponent;
class URPGAttributeSet;
class UDataAsset_EnemyStartUpData;
class UDataAsset_EnemyConfig;

/**
 * Enemy Character Base Class - 用于敌人的ASC和属性管理
 * 简化AI敌人的实现，敌人不需要持久化数据
 */
UCLASS()
class RPG_API ARPGEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ARPGEnemyCharacter();

	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Getter functions for ASC and AttributeSet
	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const { return RPGAbilitySystemComponent; }

	/** 获取玩家战斗组件 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	UEnemyCombatComponent* GetEnemyCombatComponent() const;
	
	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAttributeSet* GetRPGAttributeSet() const { return RPGAttributeSet; }

protected:
	virtual void BeginPlay() override;

private:
	// Ability System Component for enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|AbilitySystem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;

	// Attribute Set for enemy stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|AbilitySystem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URPGAttributeSet> RPGAttributeSet;

	// Startup data to grant abilities and effects on spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Startup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_EnemyStartUpData> EnemyStartUpData;

	// Enemy config for attributes (独立于 StartupData)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|EnemyConfig", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_EnemyConfig> EnemyConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	UEnemyCombatComponent* EnemyCombatComponent;

	// Initialize startup data (grant abilities and effects)
	void InitializeStartupData();

	// Initialize enemy config (apply attributes to ASC)
	void InitializeEnemyConfig();
};
