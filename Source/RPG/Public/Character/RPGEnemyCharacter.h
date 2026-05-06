// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "RPGEnemyCharacter.generated.h"

class UEnemyCombatComponent;
class URPGAbilitySystemComponent;
class URPGAttributeSet;
class UDataAsset_EnemyStartUpData;
class UDataAsset_EnemyConfig;
class UBehaviorTree;
class ARPGEnemyAIController;
class URPGEnemyUIComponent;
class URPGHealthComponent;

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

	// IPawnUIInterface implementation
	virtual URPGEnemyUIComponent* GetEnemyUIComponent() override { return EnemyUIComponent; }

	// Getter functions for ASC and AttributeSet
	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const { return RPGAbilitySystemComponent; }

	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	
	/** 获取敌人 UI 组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<URPGEnemyUIComponent> EnemyUIComponent;
	
	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAttributeSet* GetRPGAttributeSet() const { return RPGAttributeSet; }

protected:
	virtual void BeginPlay() override;

	// AI Controller 接管时初始化行为树
	virtual void PossessedBy(AController* NewController) override;

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

	// 敌人AI行为树（在编辑器中指定）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> EnemyBehaviorTree;

	// 缓存的AI控制器引用
	TWeakObjectPtr<ARPGEnemyAIController> CachedAIController;

	// Initialize startup data (grant abilities and effects)
	void InitializeStartupData();

	// Initialize enemy config (apply attributes to ASC)
	void InitializeEnemyConfig();

public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const{return EnemyCombatComponent;}
	
	// IPawnDeathInterface implementation
	virtual void OnDeathStarted_Implementation() override;
	virtual void OnDeathFinished_Implementation() override;
};
