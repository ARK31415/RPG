// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGSharedAbility_Death.h"
#include "RPGEnemyAbility_DeathBase.generated.h"

class ARPGEnemyCharacter;
class UEnemyCombatComponent;

/**
 * Enemy 死亡能力基类
 * 
 * 继承自 URPGSharedAbility_Death，为敌人角色提供特化的死亡逻辑：
 * - 停止 AI 行为树
 * - 清除仇恨/黑板数据
 * - 掉落物品
 * - 设置 LifeSpan 延迟销毁
 * 
 * 子类（或蓝图）可通过 OnEnemyDeathStarted / OnEnemyDeathFinished 进一步定制表现
 */
UCLASS(Abstract)
class RPG_API URPGEnemyAbility_DeathBase : public URPGSharedAbility_Death
{
	GENERATED_BODY()

public:
	/** 获取敌人角色 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|Death")
	ARPGEnemyCharacter* GetEnemyCharacterFromActorInfo() const;

	/** 获取敌人战斗组件 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|Death")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo() const;

protected:
	//~ URPGSharedAbility_Death interface
	virtual void StartDeathSequence_Implementation(AActor* AvatarActor) override;
	virtual void FinishDeathSequence_Implementation(AActor* AvatarActor) override;
	//~ End

	/** Enemy 死亡开始钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|Death")
	void OnEnemyDeathStarted(ARPGEnemyCharacter* EnemyCharacter);

	/** Enemy 死亡完成钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|Death")
	void OnEnemyDeathFinished(ARPGEnemyCharacter* EnemyCharacter);

	URPGEnemyAbility_DeathBase();
};
