// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Share/RPGSharedAbility_Death.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/PawnDeathInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGDeathAbility, Log, All)

URPGSharedAbility_Death::URPGSharedAbility_Death()
{
	// 通过 GameplayEvent 触发，不是通过输入
	AbilityActivationPolicy = ERPGAbilityActivationPolicy::OnTriggered;
	
	// 配置为通过 Shared.Event.Death 事件触发
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = RPGGameplayTags::Shared_Event_Death;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// 激活时添加 Dead 状态标签
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(RPGGameplayTags::Shared_Ability_Death);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(RPGGameplayTags::Shared_Status_Dead);

	// 死亡后不能再次触发
	ActivationBlockedTags.AddTag(RPGGameplayTags::Shared_Status_Dead);

	// 默认延迟
	DeathFinishDelay = 2.0f;
}

void URPGSharedAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		UE_LOG(LogRPGDeathAbility, Error, TEXT("Death GA activated but AvatarActor is null!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogRPGDeathAbility, Log, TEXT("Death GA activated for %s"), *AvatarActor->GetName());

	// 执行死亡开始序列
	StartDeathSequence(AvatarActor);

	// 延迟后完成死亡
	if (UWorld* World = AvatarActor->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DeathFinishTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo, AvatarActor]()
			{
				FinishDeathSequence(AvatarActor);
				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			}),
			DeathFinishDelay,
			false
		);
	}
}

void URPGSharedAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 清理定时器
	if (AActor* AvatarActor = ActorInfo->AvatarActor.Get())
	{
		if (UWorld* World = AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DeathFinishTimerHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URPGSharedAbility_Death::StartDeathSequence_Implementation(AActor* AvatarActor)
{
	UE_LOG(LogRPGDeathAbility, Log, TEXT("StartDeathSequence for %s"), *AvatarActor->GetName());

	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!Character)
	{
		return;
	}

	// 1. 禁用碰撞
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 2. 停止移动
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->SetMovementMode(MOVE_None);
	}

	// 3. 通知旧接口（兼容现有逻辑，如 AI Blackboard 更新）
	if (AvatarActor->GetClass()->ImplementsInterface(UPawnDeathInterface::StaticClass()))
	{
		IPawnDeathInterface::Execute_OnDeathStarted(AvatarActor);
	}
}

void URPGSharedAbility_Death::FinishDeathSequence_Implementation(AActor* AvatarActor)
{
	UE_LOG(LogRPGDeathAbility, Log, TEXT("FinishDeathSequence for %s"), *AvatarActor->GetName());

	// 通知旧接口（兼容现有逻辑，如设置 LifeSpan）
	if (AvatarActor->GetClass()->ImplementsInterface(UPawnDeathInterface::StaticClass()))
	{
		IPawnDeathInterface::Execute_OnDeathFinished(AvatarActor);
	}
}
