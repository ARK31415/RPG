// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGSharedAbility_HitReact.generated.h"

class UAnimMontage;

/**
 * 共享受击能力 (Shared HitReact Ability)
 * 
 * 设计理念：将受击反应作为一种 GameplayAbility 实现，
 * 通过 GAS 的 Tag 系统实现受击期间的能力互斥和状态管理。
 * 
 * 触发方式：通过 GameplayEvent (Shared.Event.HitReact) 触发
 * EventData 中携带受击方向 Tag（Front/Back/Left/Right）
 * 
 * 核心流程：
 * 1. 提取受击方向 → 2. StartHitReactSequence → 3. 播放方向蒙太奇 → 4. FinishHitReactSequence → EndAbility
 */
UCLASS()
class RPG_API URPGSharedAbility_HitReact : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGSharedAbility_HitReact();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 受击开始序列（可被子类/蓝图重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "HitReact")
	void StartHitReactSequence(AActor* AvatarActor, FGameplayTag HitDirection);

	/** 受击完成序列（可被子类/蓝图重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "HitReact")
	void FinishHitReactSequence(AActor* AvatarActor);

	/** 获取受击蒙太奇（可被子类/蓝图重写以动态决定蒙太奇） */
	UFUNCTION(BlueprintNativeEvent, Category = "HitReact")
	UAnimMontage* GetHitReactMontage(FGameplayTag HitDirection) const;

	/** 方向→蒙太奇映射（在编辑器中配置） */
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> DirectionalHitReactMontages;

	/** 受击蒙太奇播放速率 */
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	float HitReactPlayRate;

private:
	/** Montage 完成回调 */
	UFUNCTION()
	void OnMontageCompleted();

	/** Montage 中断回调 */
	UFUNCTION()
	void OnMontageInterrupted();
};
