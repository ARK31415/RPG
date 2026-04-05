// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "RPGBaseAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UAbilitySystemComponent;

/**
 * 动画实例基类 - 提供所有动画实例共用的基础参数和接口
 */
UCLASS()
class RPG_API URPGBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	URPGBaseAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 角色引用
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// GAS 系统引用
	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 核心运动参数（C++ 计算，蓝图读取）
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float VerticalSpeed;

	// 状态标志
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsGrounded;

private:
	void UpdateLocomotionParameters();
};
