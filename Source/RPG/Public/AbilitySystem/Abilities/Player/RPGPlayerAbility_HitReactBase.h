// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Share/RPGSharedAbility_HitReact.h"
#include "RPGPlayerAbility_HitReactBase.generated.h"

class ARPGPlayerCharacter;
class ARPGPlayerController;

/**
 * Player 受击能力基类
 * 
 * 继承自 URPGSharedAbility_HitReact，为玩家角色提供特化的受击逻辑：
 * - 摄像机震动反馈
 * - Player 特有的 UI 提示
 * 
 * 子类（或蓝图）可通过 OnPlayerHitReactStarted / OnPlayerHitReactFinished 进一步定制表现
 */
UCLASS(Abstract)
class RPG_API URPGPlayerAbility_HitReactBase : public URPGSharedAbility_HitReact
{
	GENERATED_BODY()

public:
	/** 获取玩家角色 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|HitReact")
	ARPGPlayerCharacter* GetPlayerCharacterFromActorInfo() const;

	/** 获取玩家控制器 */
	UFUNCTION(BlueprintPure, Category = "RPG|Ability|HitReact")
	ARPGPlayerController* GetPlayerControllerFromActorInfo() const;

protected:
	//~ URPGSharedAbility_HitReact interface
	virtual void StartHitReactSequence_Implementation(AActor* AvatarActor, FGameplayTag HitDirection) override;
	virtual void FinishHitReactSequence_Implementation(AActor* AvatarActor) override;
	//~ End

	/** Player 受击开始钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|HitReact")
	void OnPlayerHitReactStarted(ARPGPlayerCharacter* PlayerCharacter, FGameplayTag HitDirection);

	/** Player 受击完成钩子（蓝图可重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Ability|HitReact")
	void OnPlayerHitReactFinished(ARPGPlayerCharacter* PlayerCharacter);
};
