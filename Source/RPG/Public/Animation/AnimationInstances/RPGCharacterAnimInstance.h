// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGBaseAnimInstance.h"
#include "RPGCharacterAnimInstance.generated.h"

/**
 * 角色动画实例 - 处理移动状态和 Linked Anim Layers
 */
UCLASS()
class RPG_API URPGCharacterAnimInstance : public URPGBaseAnimInstance
{
	GENERATED_BODY()

public:
	URPGCharacterAnimInstance();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 移动状态（用于蓝图状态机 Transition Rule）
	UPROPERTY(BlueprintReadOnly, Category = "Movement State")
	bool bIsIdle;

	UPROPERTY(BlueprintReadOnly, Category = "Movement State")
	bool bIsWalking;

	UPROPERTY(BlueprintReadOnly, Category = "Movement State")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Movement State")
	bool bIsSprinting;

	// 速度阈值（可在蓝图中调整）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement Settings")
	float WalkSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement Settings")
	float RunSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement Settings")
	float SprintSpeedThreshold;

	// 步态比例（用于 Blend Space 平滑过渡）
	UPROPERTY(BlueprintReadOnly, Category = "Movement State")
	float GaitAmount;

public:
	// Linked Anim Layers 支持
	UPROPERTY(BlueprintReadOnly, Category = "Linked Layers")
	TSubclassOf<UAnimInstance> CurrentLinkedLayerClass;

	// 链接动画层（运行时切换武器动画）
	UFUNCTION(BlueprintCallable, Category = "Linked Layers")
	void LinkAnimLayer(TSubclassOf<UAnimInstance> InAnimClass);

	UFUNCTION(BlueprintCallable, Category = "Linked Layers")
	void UnlinkAnimLayer();

private:
	void UpdateMovementState();
	void UpdateGaitAmount();
};
