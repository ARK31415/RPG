// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGSharedAbility_HitReact.h"
#include "RPGEnemyAbility_HitReactBase.generated.h"

class ARPGEnemyCharacter;
class UEnemyCombatComponent;

/**
 * Enemy 受击能力基类
 * 
 * 继承自 URPGSharedAbility_HitReact，为敌人角色提供特化的受击逻辑：
 * - 暂停 AI 行为树（PauseLogic）
 * - 受击结束后恢复 AI（ResumeLogic）
 * 
 * 子类（或蓝图）可通过 OnEnemyHitReactStarted / OnEnemyHitReactFinished 进一步定制表现
 */
UCLASS(Abstract)
class RPG_API URPGEnemyAbility_HitReactBase : public URPGSharedAbility_HitReact
{
	GENERATED_BODY()

public:
	/** 获取敌人角色 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|HitReact")
	ARPGEnemyCharacter* GetEnemyCharacterFromActorInfo() const;

	/** 获取敌人战斗组件 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|HitReact")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo() const;

protected:
	//~ URPGSharedAbility_HitReact interface
	virtual void StartHitReactSequence_Implementation(AActor* AvatarActor, FGameplayTag HitDirection) override;
	virtual void FinishHitReactSequence_Implementation(AActor* AvatarActor) override;
	//~ End

	/** Enemy 受击开始钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|HitReact")
	void OnEnemyHitReactStarted(ARPGEnemyCharacter* EnemyCharacter, FGameplayTag HitDirection);

	/** Enemy 受击完成钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|HitReact")
	void OnEnemyHitReactFinished(ARPGEnemyCharacter* EnemyCharacter);
};
