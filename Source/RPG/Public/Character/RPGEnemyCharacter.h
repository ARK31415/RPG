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
class UWidgetComponent;

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

	/** 对象池激活后统一初始化入口（由对象池调用） */
	void OnEnemyActivated();

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

	/** 头顶血条 WidgetComponent（场景挂载，逻辑由 EnemyUIComponent 管理） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	/** 血条 Widget 蓝图类（可在蓝图子类中覆盖） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	friend class URPGEnemyUIComponent;  // 允许 UIComponent 访问 HealthBarWidgetClass

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
