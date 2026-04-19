// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimationInstances/RPGCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGCharacterAnimInstance, Log, All)

URPGCharacterAnimInstance::URPGCharacterAnimInstance()
	: bIsIdle(true)
	, bIsWalking(false)
	, bIsRunning(false)
	, bIsSprinting(false)
	, WalkSpeedThreshold(50.f)
	, RunSpeedThreshold(200.f)
	, SprintSpeedThreshold(500.f)
	, GaitAmount(0.f)
	, CurrentJumpState(EJumpState::None)
	, bIsJumping(false)
	, bCanJumpStart(false)
	, bCanJumpLoop(false)
	, bCanJumpLand(false)
	, bJumpAnimationFinished(false)
	, JumpStartVerticalSpeedThreshold(50.f)
	, LandDetectionDelay(0.1f)
	, TimeSinceJumpStart(0.f)
	, TimeSinceGrounded(0.f)
	, bWasFallingLastFrame(false)
	, bWasGroundedLastFrame(true)
{
}

void URPGCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[AnimInstance] NativeInitializeAnimation - Class: %s, OwningCharacter: %s"),
		*GetClass()->GetName(),
		OwningCharacter ? *OwningCharacter->GetName() : TEXT("None"));
}

void URPGCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		return;
	}

	UpdateMovementState();
	UpdateGaitAmount();
	UpdateJumpState(DeltaSeconds);
}

void URPGCharacterAnimInstance::UpdateMovementState()
{
	// 基于速度阈值判断移动状态
	bIsIdle = GroundSpeed < WalkSpeedThreshold;
	bIsWalking = GroundSpeed >= WalkSpeedThreshold && GroundSpeed < RunSpeedThreshold;
	bIsRunning = GroundSpeed >= RunSpeedThreshold && GroundSpeed < SprintSpeedThreshold;
	bIsSprinting = GroundSpeed >= SprintSpeedThreshold;
}

void URPGCharacterAnimInstance::UpdateGaitAmount()
{
	// 计算步态比例（0-1 走，1-2 跑，2-3 冲刺）
	if (bIsIdle)
	{
		GaitAmount = 0.f;
	}
	else if (bIsWalking)
	{
		// Walk: 0 -> 1
		GaitAmount = FMath::GetMappedRangeValueClamped(
			FVector2D(WalkSpeedThreshold, RunSpeedThreshold),
			FVector2D(0.f, 1.f),
			GroundSpeed
		);
	}
	else if (bIsRunning)
	{
		// Run: 1 -> 2
		GaitAmount = FMath::GetMappedRangeValueClamped(
			FVector2D(RunSpeedThreshold, SprintSpeedThreshold),
			FVector2D(1.f, 2.f),
			GroundSpeed
		);
	}
	else // Sprinting
	{
		GaitAmount = 2.f;
	}
}

void URPGCharacterAnimInstance::LinkAnimLayer(TSubclassOf<UAnimInstance> InAnimClass)
{
	if (InAnimClass && InAnimClass != CurrentLinkedLayerClass)
	{
		CurrentLinkedLayerClass = InAnimClass;
		LinkAnimClassLayers(InAnimClass);
	}
}

void URPGCharacterAnimInstance::UnlinkAnimLayer()
{
	if (CurrentLinkedLayerClass)
	{
		UnlinkAnimClassLayers(CurrentLinkedLayerClass);
		CurrentLinkedLayerClass = nullptr;
	}
}

// ========== 跳跃状态系统 ==========

void URPGCharacterAnimInstance::UpdateJumpState(float DeltaSeconds)
{
	// 更新计时器
	if (bIsFalling)
	{
		TimeSinceJumpStart += DeltaSeconds;
		TimeSinceGrounded = 0.f; // 在空中时重置地面计时
	}
	else
	{
		TimeSinceGrounded += DeltaSeconds;
		TimeSinceJumpStart = 0.f; // 在地面时重置跳跃计时
	}

	// 检测状态变化
	bool bJustStartedFalling = !bWasFallingLastFrame && bIsFalling;
	bool bJustLanded = bWasFallingLastFrame && bIsGrounded;

	// 根据当前状态更新逻辑
	switch (CurrentJumpState)
	{
		case EJumpState::None:
			HandleJumpStart();
			break;
			
		case EJumpState::Start:
			HandleJumpLoop();
			break;
			
		case EJumpState::Loop:
			HandleJumpLand();
			break;
			
		case EJumpState::Land:
			// 落地动画播完后回到 None
		//UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[EJumpState::Land] bJumpAnimationFinished: %d, bIsGrounded: %d"), bJumpAnimationFinished, bIsGrounded);
			if (bJumpAnimationFinished && bIsGrounded)
			{
				UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[JumpState] Transition: Land -> None (Reset complete)"));
				CurrentJumpState = EJumpState::None;
				bIsJumping = false;
				TimeSinceJumpStart = 0.f;
				TimeSinceGrounded = 0.f;
				bJumpAnimationFinished = false;
				bWasFallingLastFrame = false;  // 重置帧状态
				bWasGroundedLastFrame = true;
			}
			else
			{
				// Land 状态下累加 TimeSinceGrounded 用于判断动画播放进度
				TimeSinceGrounded += DeltaSeconds;
			}
			break;
	}

	// 记录上一帧状态
	bWasFallingLastFrame = bIsFalling;
	bWasGroundedLastFrame = bIsGrounded;
}

void URPGCharacterAnimInstance::HandleJumpStart()
{
	// 从 None 过渡到 Start 的条件
	// 1. 刚进入 falling 状态
	// 2. 垂直速度超过阈值（正在上升）
	bCanJumpStart = bIsFalling && VerticalSpeed > JumpStartVerticalSpeedThreshold;
	/*UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[JumpState] HandleJumpStart - bCanJumpStart: %d, bIsFalling: %d, VerticalSpeed: %.1f, Threshold: %.1f"), 
		bCanJumpStart, bIsFalling, VerticalSpeed, JumpStartVerticalSpeedThreshold);*/
	bCanJumpLoop = false;
	bCanJumpLand = false;
	bIsJumping = false;

	if (bCanJumpStart)
	{
		CurrentJumpState = EJumpState::Start;
		bIsJumping = true;
		TimeSinceJumpStart = 0.f;
		bJumpAnimationFinished = false;
		
		UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[JumpState] Transition: None -> Start (VerticalSpeed: %.1f)"),
			VerticalSpeed);
	}
}

void URPGCharacterAnimInstance::HandleJumpLoop()
{
	// Start 状态下的逻辑
	bCanJumpStart = false;
	bCanJumpLand = false;
	bIsJumping = true;

	// 过渡到 Loop 的条件（二选一）：
	// 方案A：起跳动画播放了一段时间（推荐 0.3-0.5 秒）
	// 方案B：到达最高点（垂直速度接近 0 或变为负值）
	bool bShouldTransitionToLoop = TimeSinceJumpStart >= 0.4f || VerticalSpeed <= 0.f;
	
	bCanJumpLoop = bShouldTransitionToLoop;

	if (bCanJumpLoop)
	{
		CurrentJumpState = EJumpState::Loop;
		bJumpAnimationFinished = false;
		
		UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[JumpState] Transition: Start -> Loop (Time: %.2f, VerticalSpeed: %.1f)"),
			TimeSinceJumpStart, VerticalSpeed);
	}
}

void URPGCharacterAnimInstance::HandleJumpLand()
{
	// Loop 状态下的逻辑
	bCanJumpStart = false;
	bCanJumpLoop = false;
	bIsJumping = true;

	// 过渡到 Land 的条件：接触地面
	bCanJumpLand = bIsGrounded && TimeSinceGrounded >= LandDetectionDelay;

	if (bCanJumpLand)
	{
		CurrentJumpState = EJumpState::Land;
		bJumpAnimationFinished = false;
		
		UE_LOG(LogRPGCharacterAnimInstance, Log, TEXT("[JumpState] Transition: Loop -> Land (Grounded for: %.2fs)"),
			TimeSinceGrounded);
	}
}
