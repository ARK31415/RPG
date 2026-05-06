// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGSharedAbility_Death.h"
#include "RPGPlayerAbility_DeathBase.generated.h"

class ARPGPlayerCharacter;
class ARPGPlayerController;

/**
 * Player 死亡能力基类
 * 
 * 继承自 URPGSharedAbility_Death，为玩家角色提供特化的死亡逻辑：
 * - 禁用玩家输入
 * - 通知 PlayerController
 * - 触发死亡 UI / 重生流程
 * 
 * 子类（或蓝图）可通过 OnPlayerDeathStarted / OnPlayerDeathFinished 进一步定制表现
 */
UCLASS(Abstract)
class RPG_API URPGPlayerAbility_DeathBase : public URPGSharedAbility_Death
{
	GENERATED_BODY()

public:
	/** 获取玩家角色 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|Death")
	ARPGPlayerCharacter* GetPlayerCharacterFromActorInfo() const;

	/** 获取玩家控制器 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|Death")
	ARPGPlayerController* GetPlayerControllerFromActorInfo() const;

protected:
	//~ URPGSharedAbility_Death interface
	virtual void StartDeathSequence_Implementation(AActor* AvatarActor) override;
	virtual void FinishDeathSequence_Implementation(AActor* AvatarActor) override;
	//~ End

	/** Player 死亡开始钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|Death")
	void OnPlayerDeathStarted(ARPGPlayerCharacter* PlayerCharacter);

	/** Player 死亡完成钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|Death")
	void OnPlayerDeathFinished(ARPGPlayerCharacter* PlayerCharacter);
};
