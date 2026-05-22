// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Share/RPGSharedAbility_Death.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "Interface/PawnDeathInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "RPGDebugHelper.h"

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

	// 默认蒙太奇播放速率
	DeathMontagePlayRate = 1.0f;
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

	UE_LOG(LogRPGDeathAbility, Log, TEXT("[Death-SharedDeathGA] Activate - %s"), *AvatarActor->GetName());
	//Debug::Print(FString::Printf(TEXT("[Death-SharedDeathGA] Activate - %s"), *AvatarActor->GetName()));

	// 执行死亡开始序列
	StartDeathSequence(AvatarActor);

	// 从 TriggerEventData 提取死亡方向 Tag（复用 HitReact 方向标签）
	FGameplayTag DeathDirection;
	if (TriggerEventData)
	{
		if (TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Front))
		{
			DeathDirection = RPGGameplayTags::Shared_Status_HitReact_Front;
		}
		else if (TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Back))
		{
			DeathDirection = RPGGameplayTags::Shared_Status_HitReact_Back;
		}
		else if (TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Left))
		{
			DeathDirection = RPGGameplayTags::Shared_Status_HitReact_Left;
		}
		else if (TriggerEventData->InstigatorTags.HasTag(RPGGameplayTags::Shared_Status_HitReact_Right))
		{
			DeathDirection = RPGGameplayTags::Shared_Status_HitReact_Right;
		}
	}

	// 播放死亡蒙太奇（内部包含 fallback Timer 兜底逻辑）
	PlayDeathMontage(DeathDirection);
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
	UE_LOG(LogRPGDeathAbility, Log, TEXT("[Death-SharedDeathGA] StartDeathSeq - %s"), *AvatarActor->GetName());
	//Debug::Print(FString::Printf(TEXT("[Death-SharedDeathGA] StartDeathSeq - %s"), *AvatarActor->GetName()));

	// 通知 Character 处理死亡表现逻辑（碰撞、移动、动画、AI 等）
	if (AvatarActor->GetClass()->ImplementsInterface(UPawnDeathInterface::StaticClass()))
	{
		IPawnDeathInterface::Execute_OnDeathStarted(AvatarActor);
	}
}

void URPGSharedAbility_Death::FinishDeathSequence_Implementation(AActor* AvatarActor)
{
	UE_LOG(LogRPGDeathAbility, Log, TEXT("[Death-SharedDeathGA] FinishDeathSeq - %s, IsHidden=%s"), *AvatarActor->GetName(), AvatarActor->IsHidden() ? TEXT("true") : TEXT("false"));
	//Debug::Print(FString::Printf(TEXT("[Death-SharedDeathGA] FinishDeathSeq - %s, IsHidden=%s"), *AvatarActor->GetName(), AvatarActor->IsHidden() ? TEXT("true") : TEXT("false")));

	// 通知旧接口（兼容现有逻辑，如设置 LifeSpan）
	if (AvatarActor->GetClass()->ImplementsInterface(UPawnDeathInterface::StaticClass()))
	{
		IPawnDeathInterface::Execute_OnDeathFinished(AvatarActor);
	}
}

UAnimMontage* URPGSharedAbility_Death::GetDeathMontage_Implementation(FGameplayTag DeathDirection) const
{
	// 优先查找精确方向匹配
	if (DeathDirection.IsValid())
	{
		if (const TObjectPtr<UAnimMontage>* FoundMontage = DirectionalDeathMontages.Find(DeathDirection))
		{
			return *FoundMontage;
		}
	}

	// 回退到默认死亡蒙太奇
	return DefaultDeathMontage;
}

void URPGSharedAbility_Death::PlayDeathMontage(FGameplayTag DeathDirection)
{
	UAnimMontage* MontageToPlay = GetDeathMontage(DeathDirection);

	if (MontageToPlay)
	{
		UE_LOG(LogRPGDeathAbility, Log, TEXT("[Death-SharedDeathGA] PlayMontage - %s, Dir=%s"), *MontageToPlay->GetName(), *DeathDirection.ToString());
		Debug::Log(FString::Printf(TEXT("[Death-SharedDeathGA] PlayMontage - %s, Dir=%s"), *MontageToPlay->GetName(), *DeathDirection.ToString()));

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			DeathMontagePlayRate,
			FName(),
			0.0f,
			true
		);

		MontageTask->OnCompleted.AddDynamic(this, &URPGSharedAbility_Death::OnDeathMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &URPGSharedAbility_Death::OnDeathMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &URPGSharedAbility_Death::OnDeathMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &URPGSharedAbility_Death::OnDeathMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// Fallback：没有配置蒙太奇时，使用 Timer 延迟后完成死亡
		UE_LOG(LogRPGDeathAbility, Warning, TEXT("[Death-SharedDeathGA] NoMontage! Fallback %.1fs"), DeathFinishDelay);
		//Debug::PrintWarning(FString::Printf(TEXT("[Death-SharedDeathGA] NoMontage! Fallback %.1fs"), DeathFinishDelay));

		if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			if (UWorld* World = AvatarActor->GetWorld())
			{
				World->GetTimerManager().SetTimer(
					DeathFinishTimerHandle,
					FTimerDelegate::CreateWeakLambda(this, [this]()
					{
						if (AActor* FallbackAvatar = GetAvatarActorFromActorInfo())
						{
							FinishDeathSequence(FallbackAvatar);
						}
						if (IsActive())
						{
							EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
						}
					}),
					DeathFinishDelay,
					false
				);
			}
		}
	}
}

void URPGSharedAbility_Death::OnDeathMontageCompleted()
{
	UE_LOG(LogRPGDeathAbility, Log, TEXT("Death montage completed."));

	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		// 动画播放完毕立即隐藏 Actor（避免静止尸体可见）
		AvatarActor->SetActorHiddenInGame(true);
		UE_LOG(LogRPGDeathAbility, Log, TEXT("[Death-SharedDeathGA] MontageCompleted - %s, SetHidden=true"), *AvatarActor->GetName());
		//Debug::Print(FString::Printf(TEXT("[Death-SharedDeathGA] MontageCompleted - %s, SetHidden=true"), *AvatarActor->GetName()));

		// 继续执行后续死亡逻辑（经验奖励、掉落物、LifeSpan 等）
		FinishDeathSequence(AvatarActor);
	}

	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void URPGSharedAbility_Death::OnDeathMontageInterrupted()
{
	UE_LOG(LogRPGDeathAbility, Log, TEXT("Death montage interrupted."));

	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		// 动画被中断也立即隐藏 Actor
		AvatarActor->SetActorHiddenInGame(true);
		UE_LOG(LogRPGDeathAbility, Warning, TEXT("[Death-SharedDeathGA] MontageInterrupted - %s, SetHidden=true"), *AvatarActor->GetName());
		//Debug::PrintWarning(FString::Printf(TEXT("[Death-SharedDeathGA] MontageInterrupted - %s, SetHidden=true"), *AvatarActor->GetName()));

		FinishDeathSequence(AvatarActor);
	}

	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
