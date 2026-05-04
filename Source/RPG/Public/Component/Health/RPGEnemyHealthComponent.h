// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Health/RPGHealthComponent.h"
#include "RPGEnemyHealthComponent.generated.h"

/**
 * Enemy Health Component
 * 职责：敌人特有的健康逻辑（死亡动画、掉落、经验奖励等）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGEnemyHealthComponent : public URPGHealthComponent
{
	GENERATED_BODY()

public:
	URPGEnemyHealthComponent();

	/** 设置是否显示死亡动画 */
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetPlayDeathAnimation(bool bPlayAnim) { bPlayDeathAnimation = bPlayAnim; }

	/** 获取是否播放死亡动画 */
	UFUNCTION(BlueprintPure, Category="Health")
	bool ShouldPlayDeathAnimation() const { return bPlayDeathAnimation; }

protected:
	virtual void StartDeath() override;
	virtual void FinishDeath() override;

private:
	/** 是否播放死亡动画 */
	UPROPERTY(EditDefaultsOnly, Category="Health|Animation")
	bool bPlayDeathAnimation;

	/** 死亡动画持续时间 */
	UPROPERTY(EditDefaultsOnly, Category="Health|Animation")
	float DeathAnimationDuration;

	/** 是否在死亡时销毁 Actor */
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	bool bDestroyOnDeath;

	/** 死亡后销毁延迟 */
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float DestroyDelay;
};
