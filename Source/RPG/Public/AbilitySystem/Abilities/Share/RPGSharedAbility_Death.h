// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGSharedAbility_Death.generated.h"

class UAnimMontage;

/**
 * 共享死亡能力 (Shared Death Ability)
 * 
 * 设计理念：将死亡过程作为一种 GameplayAbility 实现，而非直接写在 Character 中。
 * 好处：
 * - 死亡过程可被 GameplayTag 控制（如 BOSS 阶段转换时不可杀死）
 * - 支持不同角色用不同死亡 GA（通过 DataAsset 配置）
 * - 网络复制由 GAS 框架自动处理
 * - 符合 "一切行为皆 Ability" 的 GAS 设计哲学
 * 
 * 触发方式：通过 GameplayEvent (Shared.Event.Death) 触发
 * 由 HealthComponent 在生命值归零时发送该事件
 */
UCLASS()
class RPG_API URPGSharedAbility_Death : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGSharedAbility_Death();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 死亡开始序列（可被蓝图重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "Death")
	void StartDeathSequence(AActor* AvatarActor);

	/** 死亡完成序列（可被蓝图重写） */
	UFUNCTION(BlueprintNativeEvent, Category = "Death")
	void FinishDeathSequence(AActor* AvatarActor);

	/** 获取死亡蒙太奇（可被子类/蓝图重写以动态决定蒙太奇） */
	UFUNCTION(BlueprintNativeEvent, Category = "Death")
	UAnimMontage* GetDeathMontage(FGameplayTag DeathDirection) const;

	/** 方向→死亡蒙太奇映射（在编辑器中配置，如 Front/Back/Left/Right） */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> DirectionalDeathMontages;

	/** 默认死亡蒙太奇（兜底，当未匹配到具体方向时使用） */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DefaultDeathMontage;

	/** 死亡蒙太奇播放速率 */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontagePlayRate;

private:
	/** 死亡完成延迟计时器 */
	FTimerHandle DeathFinishTimerHandle;

	/** 死亡完成延迟时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathFinishDelay;

	/** 播放死亡蒙太奇并绑定回调 */
	void PlayDeathMontage(FGameplayTag DeathDirection);

	/** 蒙太奇完成回调 */
	UFUNCTION()
	void OnDeathMontageCompleted();

	/** 蒙太奇中断回调 */
	UFUNCTION()
	void OnDeathMontageInterrupted();
};
