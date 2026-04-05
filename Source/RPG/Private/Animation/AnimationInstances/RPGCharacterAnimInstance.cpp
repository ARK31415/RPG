// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimationInstances/RPGCharacterAnimInstance.h"

URPGCharacterAnimInstance::URPGCharacterAnimInstance()
	: bIsIdle(true)
	, bIsWalking(false)
	, bIsRunning(false)
	, bIsSprinting(false)
	, WalkSpeedThreshold(50.f)
	, RunSpeedThreshold(200.f)
	, SprintSpeedThreshold(500.f)
	, GaitAmount(0.f)
{
}

void URPGCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		return;
	}

	UpdateMovementState();
	UpdateGaitAmount();
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
