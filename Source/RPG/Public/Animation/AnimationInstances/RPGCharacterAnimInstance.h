// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGBaseAnimInstance.h"
#include "RPGCharacterAnimInstance.generated.h"

class ACharacter;

/**
 * 跳跃状态枚举
 */
UENUM(BlueprintType)
enum class EJumpState : uint8
{
	None		UMETA(DisplayName = "无跳跃"),
	Start		UMETA(DisplayName = "起跳"),
	Loop		UMETA(DisplayName = "滞空"),
	Land		UMETA(DisplayName = "落地")
};

/**
 * 角色动画实例 - 处理移动状态和 Linked Anim Layers
 */
UCLASS()
class RPG_API URPGCharacterAnimInstance : public URPGBaseAnimInstance
{
	GENERATED_BODY()

public:
	URPGCharacterAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 移动状态（用于蓝图状态机 Transition Rule）
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Movement State")
	bool bIsIdle;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Movement State")
	bool bIsWalking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Movement State")
	bool bIsRunning;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Movement State")
	bool bIsSprinting;

	// ========== 跳跃状态系统（C++ 计算，蓝图读取） ==========
	
	/** 当前跳跃状态（None/Start/Loop/Land） */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Jump State")
	EJumpState CurrentJumpState;

	/** 是否正在跳跃（任何跳跃阶段） */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Jump State")
	bool bIsJumping;

	/** 是否可以过渡到起跳状态 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Jump State")
	bool bCanJumpStart;

	/** 是否可以过渡到滞空状态 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Jump State")
	bool bCanJumpLoop;

	/** 是否可以过渡到落地状态 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Jump State")
	bool bCanJumpLand;

	/** 跳跃动画是否播放完成（用于状态退出判断） */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Jump State")
	bool bJumpAnimationFinished;

	/** 垂直速度阈值（判断起跳） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Jump Settings")
	float JumpStartVerticalSpeedThreshold;

	/** 落地检测延迟（防止瞬时触地误判） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Jump Settings")
	float LandDetectionDelay;

	// 速度阈值（可在蓝图中调整）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Movement Settings")
	float WalkSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Movement Settings")
	float RunSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimData|Movement Settings")
	float SprintSpeedThreshold;

	// 步态比例（用于 Blend Space 平滑过渡）
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Movement State")
	float GaitAmount;

public:
	// Linked Anim Layers 支持
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Linked Layers")
	TSubclassOf<UAnimInstance> CurrentLinkedLayerClass;

	// 链接动画层（运行时切换武器动画）
	UFUNCTION(BlueprintCallable, Category = "Linked Layers")
	void LinkAnimLayer(TSubclassOf<UAnimInstance> InAnimClass);

	UFUNCTION(BlueprintCallable, Category = "Linked Layers")
	void UnlinkAnimLayer();

private:
	void UpdateMovementState();
	void UpdateGaitAmount();
	void UpdateJumpState(float DeltaSeconds);
	void HandleJumpStart();
	void HandleJumpLoop();
	void HandleJumpLand();
	
	// 跳跃状态内部跟踪
	float TimeSinceJumpStart;
	float TimeSinceGrounded;
	bool bWasFallingLastFrame;
	bool bWasGroundedLastFrame;
};
