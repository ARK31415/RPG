// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGAbility_AttackCombo.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 连招基类（Share层）
 * 所有连招类型的通用逻辑：Montage播放、连招计数、窗口期管理
 * 玩家和敌人均可继承使用
 */
UCLASS(Abstract)
class RPG_API URPGAbility_AttackCombo : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGAbility_AttackCombo();

protected:
	// ========== 连招数据配置（子类CDO中设置） ==========

	/** 连招动画序列，按段数顺序排列 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	/** 最大连招段数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	int32 MaxComboCount;

	/** 连招输入窗口期时间（秒） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float ComboWindowTime;

	// ========== 运行时状态 ==========

	/** 当前连招计数（从0开始） */
	int32 CurrentComboIndex;

	/** 是否收到了连招输入（窗口期内按键） */
	bool bPendingComboInput;

	/** 当前播放的Montage任务 */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	// ========== Ability生命周期 ==========

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// ========== 连招核心逻辑（子类可重写） ==========

	/** 播放当前连招段的Montage */
	virtual void PlayCurrentComboMontage();

	/** 连招输入检测：收到输入时标记pending */
	virtual void OnComboInputReceived();

	/** 判断是否可以继续连招 */
	virtual bool CanContinueCombo() const;

	/** 推进到下一段连招 */
	virtual void AdvanceCombo();

	/** 重置连招状态 */
	virtual void ResetComboState();

	/** 获取当前段的Montage */
	UAnimMontage* GetCurrentComboMontage() const;

	// ========== Montage回调（虚函数，子类可重写） ==========

	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageBlendOut();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnMontageCancelled();

	// ========== 连招窗口期 ==========

	/** 连招窗口期开启（由AnimNotify触发的GameplayEvent驱动） */
	UFUNCTION()
	virtual void OnComboWindowOpen(FGameplayEventData Payload);

	/** 连招窗口期关闭（由AnimNotify触发的GameplayEvent驱动） */
	UFUNCTION()
	virtual void OnComboWindowClose(FGameplayEventData Payload);

private:
	/** 等待连招窗口期事件 */
	void WaitForComboWindowEvents();
};
