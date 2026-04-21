// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimationInstances/Enemy/RPGEnemyAnimInstanceBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyAnimInstance, Log, All)

URPGEnemyAnimInstanceBase::URPGEnemyAnimInstanceBase()
	: bIsIdle(true)
	, MovementSpeed(0.f)
	, IdleMoveSpeedThreshold(50.f)
	, bIsHitReacting(false)
	, HitReactDirection(EEnemyHitReactDirection::Front)
	, bIsDead(false)
	, bIsAttacking(false)
{
}

void URPGEnemyAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	UE_LOG(LogEnemyAnimInstance, Log, TEXT("[EnemyAnimInstance] NativeInitializeAnimation - Class: %s, OwningCharacter: %s"),
		*GetClass()->GetName(),
		OwningCharacter ? *OwningCharacter->GetName() : TEXT("None"));

	// 初始化战斗状态（后续可通过GAS Tag回调更新）
	bIsHitReacting = false;
	bIsDead = false;
	bIsAttacking = false;
}

void URPGEnemyAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		return;
	}

	UpdateMovementState();
	UpdateCombatState();
}

void URPGEnemyAnimInstanceBase::UpdateMovementState()
{
	// 简化版移动状态判断：Idle / Moving
	bIsIdle = GroundSpeed < IdleMoveSpeedThreshold;
	bIsMoving = !bIsIdle;

	// 计算归一化移动速度（0-1），用于 Blend Space
	// 假设最大移动速度为 600（可根据敌人类型调整）
	MovementSpeed = FMath::GetMappedRangeValueClamped(
		FVector2D(IdleMoveSpeedThreshold, 600.f),
		FVector2D(0.f, 1.f),
		GroundSpeed
	);
}

void URPGEnemyAnimInstanceBase::UpdateCombatState()
{
	// 战斗状态更新逻辑
	// 注意：这里不能直接访问ASC的Tag（NativeThreadSafeUpdateAnimation 是线程安全的）
	// 战斗状态应该通过以下方式更新：
	// 1. 在 NativeUpdateAnimation（非线程安全）中读取 ASC Tag
	// 2. 或者通过动画蓝图从Character获取状态
	// 3. 或者通过GAS GameplayCue通知

	// 当前版本：预留接口，具体状态由动画蓝图或Character同步
	// 后续可以在这里添加基于 ASC Tag 的状态判断
}
