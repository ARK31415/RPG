// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Share/RPGSharedAbility_HitReact.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGHitReactAbility, Log, All)

URPGSharedAbility_HitReact::URPGSharedAbility_HitReact()
{
	// 通过 GameplayEvent 触发
	AbilityActivationPolicy = ERPGAbilityActivationPolicy::OnTriggered;

	// 配置为通过 Shared.Event.HitReact 事件触发
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = RPGGameplayTags::Shared_Event_HitReact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// 激活时添加 HitReact 能力标签
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(RPGGameplayTags::Shared_Ability_HitReact);
	SetAssetTags(AssetTags);

	// 死亡时不触发受击
	ActivationBlockedTags.AddTag(RPGGameplayTags::Shared_Status_Dead);

	// 默认播放速率
	HitReactPlayRate = 1.0f;
}

void URPGSharedAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		UE_LOG(LogRPGHitReactAbility, Error, TEXT("HitReact GA activated but AvatarActor is null!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 从 EventData 中提取受击方向 Tag
	FGameplayTag HitDirection;
	if (TriggerEventData && TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Front))
	{
		HitDirection = RPGGameplayTags::Shared_Status_HitReact_Front;
	}
	else if (TriggerEventData && TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Back))
	{
		HitDirection = RPGGameplayTags::Shared_Status_HitReact_Back;
	}
	else if (TriggerEventData && TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Left))
	{
		HitDirection = RPGGameplayTags::Shared_Status_HitReact_Left;
	}
	else if (TriggerEventData && TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Right))
	{
		HitDirection = RPGGameplayTags::Shared_Status_HitReact_Right;
	}
	else
	{
		// 默认前方受击
		HitDirection = RPGGameplayTags::Shared_Status_HitReact_Front;
	}

	UE_LOG(LogRPGHitReactAbility, Log, TEXT("HitReact GA activated for %s, Direction: %s"),
		*AvatarActor->GetName(), *HitDirection.ToString());

	// 执行受击开始序列
	StartHitReactSequence(AvatarActor, HitDirection);

	// 获取并播放受击蒙太奇
	UAnimMontage* MontageToPlay = GetHitReactMontage(HitDirection);
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			HitReactPlayRate,
			FName(),
			0.0f,
			true
		);

		MontageTask->OnCompleted.AddDynamic(this, &URPGSharedAbility_HitReact::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &URPGSharedAbility_HitReact::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &URPGSharedAbility_HitReact::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &URPGSharedAbility_HitReact::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogRPGHitReactAbility, Warning, TEXT("No HitReact montage found for direction: %s, ending ability immediately."), *HitDirection.ToString());
		FinishHitReactSequence(AvatarActor);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void URPGSharedAbility_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URPGSharedAbility_HitReact::StartHitReactSequence_Implementation(AActor* AvatarActor, FGameplayTag HitDirection)
{
	UE_LOG(LogRPGHitReactAbility, Log, TEXT("StartHitReactSequence for %s"), *AvatarActor->GetName());

	// 停止当前移动
	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->StopMovementImmediately();
		}
	}
}

void URPGSharedAbility_HitReact::FinishHitReactSequence_Implementation(AActor* AvatarActor)
{
	UE_LOG(LogRPGHitReactAbility, Log, TEXT("FinishHitReactSequence for %s"), *AvatarActor->GetName());
}

UAnimMontage* URPGSharedAbility_HitReact::GetHitReactMontage_Implementation(FGameplayTag HitDirection) const
{
	if (const TObjectPtr<UAnimMontage>* FoundMontage = DirectionalHitReactMontages.Find(HitDirection))
	{
		return *FoundMontage;
	}

	// 回退：如果没找到精确方向，尝试 Front
	if (HitDirection != RPGGameplayTags::Shared_Status_HitReact_Front)
	{
		if (const TObjectPtr<UAnimMontage>* FallbackMontage = DirectionalHitReactMontages.Find(RPGGameplayTags::Shared_Status_HitReact_Front))
		{
			return *FallbackMontage;
		}
	}

	return nullptr;
}

void URPGSharedAbility_HitReact::OnMontageCompleted()
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		FinishHitReactSequence(AvatarActor);
	}

	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void URPGSharedAbility_HitReact::OnMontageInterrupted()
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		FinishHitReactSequence(AvatarActor);
	}

	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
