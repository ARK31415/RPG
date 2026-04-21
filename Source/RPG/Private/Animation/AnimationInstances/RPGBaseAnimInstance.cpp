// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimationInstances/RPGBaseAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "RPGFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGBaseAnimInstance,All,All)

URPGBaseAnimInstance::URPGBaseAnimInstance()
	: GroundSpeed(0.f)
	, Direction(0.f)
	, Velocity(FVector::ZeroVector)
	, VerticalSpeed(0.f)
	, bIsMoving(false)
	, bIsFalling(false)
	, bIsGrounded(true)
{
}

void URPGBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 获取角色引用
	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		MovementComponent = OwningCharacter->GetCharacterMovement();
		
		// Try to get ASC from character first (for enemies)
		AbilitySystemComponent = OwningCharacter->FindComponentByClass<UAbilitySystemComponent>();
		
		// If not found, try to get from PlayerState (for players)
		if (!AbilitySystemComponent)
		{
			if (APlayerState* PlayerState = OwningCharacter->GetPlayerState())
			{
				IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState);
				if (ASI)
				{
					AbilitySystemComponent = ASI->GetAbilitySystemComponent();
				}
			}
		}
	}
}

void URPGBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !MovementComponent)
	{
		return;
	}

	UpdateLocomotionParameters();
}

bool URPGBaseAnimInstance::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
	if(APawn* OwningPawn = TryGetPawnOwner())
	{
		return URPGFunctionLibrary::NativeDoesActorHasTag(OwningPawn, TagToCheck);
	}

	return false;
}

void URPGBaseAnimInstance::UpdateLocomotionParameters()
{
	// 获取速度
	Velocity = OwningCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();
	//UE_LOG(LogRPGBaseAnimInstance, Log, TEXT("GroundSpeed: %f"), GroundSpeed);
	VerticalSpeed = Velocity.Z;
	//UE_LOG(LogRPGBaseAnimInstance, Log, TEXT("VerticalSpeed: %f"), VerticalSpeed);

	// 计算移动方向（相对于角色朝向，-180 到 180）
	if (GroundSpeed > 3.f)
	{
		const FRotator CharacterRotation = OwningCharacter->GetActorRotation();
		const FRotator VelocityRotation = Velocity.ToOrientationRotator();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, CharacterRotation).Yaw;
		//UE_LOG(LogRPGBaseAnimInstance, Log, TEXT("Direction: %f"), Direction);
	}

	// 更新状态标志
	bIsMoving = GroundSpeed > 3.f;
	bIsFalling = MovementComponent->IsFalling();
	bIsGrounded = !bIsFalling;
}
