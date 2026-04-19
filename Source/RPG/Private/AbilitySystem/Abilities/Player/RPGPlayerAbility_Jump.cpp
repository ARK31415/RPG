// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_Jump.h"
#include "Character/RPGPlayerCharacter.h"
#include "RPGGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGJumpAbility, Log, All)

URPGPlayerAbility_Jump::URPGPlayerAbility_Jump()
{
	// 实例策略：每个Actor一个实例
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 网络策略：本地预测
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// 激活时需要跳跃状态Tag不存在的条件（防止空中重复激活）
	ActivationBlockedTags.AddTag(RPGGameplayTags::Player_Status_Jumping);
}

void URPGPlayerAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
											  const FGameplayAbilityActorInfo* ActorInfo, 
											  const FGameplayAbilityActivationInfo ActivationInfo, 
											  const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGJumpAbility, Log, TEXT("[JumpAbility] ActivateAbility - Actor: %s"), 
		ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("None"));

	// 应用跳跃状态Tag
	ApplyJumpingTag();
	
	// 执行跳跃
	PerformJump();
	
	// 注意：不调用Super::ActivateAbility，因为我们不需要等待动画结束
	// 跳跃是瞬时动作，执行完毕后立即结束Ability
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void URPGPlayerAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, 
										const FGameplayAbilityActorInfo* ActorInfo, 
										const FGameplayAbilityActivationInfo ActivationInfo, 
										bool bReplicateEndAbility, 
										bool bWasCancelled)
{
	UE_LOG(LogRPGJumpAbility, Log, TEXT("[JumpAbility] EndAbility - Actor: %s, bWasCancelled: %d"), 
		ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("None"),
		bWasCancelled);

	// 移除跳跃状态Tag
	RemoveJumpingTag();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool URPGPlayerAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, 
												 const FGameplayAbilityActorInfo* ActorInfo, 
												 const FGameplayTagContainer* SourceTags, 
												 const FGameplayTagContainer* TargetTags, 
												 FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		UE_LOG(LogRPGJumpAbility, Verbose, TEXT("[JumpAbility] CanActivateAbility failed: Super check failed"));
		return false;
	}

	// 检查跳跃条件（地面或土狼时间内）
	if (!CanJump())
	{
		UE_LOG(LogRPGJumpAbility, Verbose, 
			TEXT("[JumpAbility] CanActivateAbility failed: Cannot jump"));
		return false;
	}

	return true;
}



bool URPGPlayerAbility_Jump::CanJump() const
{
	ARPGPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return false;
	
	// 委托给Character的原生CanJump，已集成土狼时间
	const bool bCanJump = PlayerCharacter->CanJump();
	
	UE_LOG(LogRPGJumpAbility, Verbose, TEXT("[JumpAbility] CanJump: %d"), bCanJump);
	
	return bCanJump;
}

void URPGPlayerAbility_Jump::PerformJump()
{
	ARPGPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		UE_LOG(LogRPGJumpAbility, Error, TEXT("[JumpAbility] PerformJump failed: PlayerCharacter is null"));
		return;
	}
	
	UE_LOG(LogRPGJumpAbility, Log, TEXT("[JumpAbility] PerformJump - Actor: %s, Location: %s"), 
		*PlayerCharacter->GetName(),
		*PlayerCharacter->GetActorLocation().ToString());
	
	// 调用UE原生跳跃
	PlayerCharacter->Jump();
}

void URPGPlayerAbility_Jump::ApplyJumpingTag()
{
	if (URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(RPGGameplayTags::Player_Status_Jumping);
		UE_LOG(LogRPGJumpAbility, Log, TEXT("[JumpAbility] Applied Player_Status_Jumping tag"));
	}
}

void URPGPlayerAbility_Jump::RemoveJumpingTag()
{
	if (URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(RPGGameplayTags::Player_Status_Jumping);
		UE_LOG(LogRPGJumpAbility, Log, TEXT("[JumpAbility] Removed Player_Status_Jumping tag"));
	}
}

ARPGPlayerCharacter* URPGPlayerAbility_Jump::GetPlayerCharacter() const
{
	// 使用CurrentActorInfo而不是ActorInfo（成员变量可能未初始化）
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		return Cast<ARPGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	}
	return nullptr;
}
