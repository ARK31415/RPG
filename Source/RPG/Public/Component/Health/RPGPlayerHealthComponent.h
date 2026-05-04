// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Health/RPGHealthComponent.h"
#include "RPGPlayerHealthComponent.generated.h"

/**
 * Player Health Component
 * 职责：玩家特有的健康逻辑（复活、无敌帧、伤害免疫等）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGPlayerHealthComponent : public URPGHealthComponent
{
	GENERATED_BODY()

public:
	URPGPlayerHealthComponent();

	/** 设置无敌状态（受伤后短暂无敌） */
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetInvincible(bool bInvincible);

	/** 是否处于无敌状态 */
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsInvincible() const { return bIsInvincible; }

	/** 复活玩家 */
	UFUNCTION(BlueprintCallable, Category="Health")
	void Revive(float HealthPercent = 0.5f);

protected:
	virtual void StartDeath() override;
	virtual void FinishDeath() override;

private:
	/** 是否无敌 */
	bool bIsInvincible;

	/** 无敌时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category="Health")
	float InvincibleDuration;
};
