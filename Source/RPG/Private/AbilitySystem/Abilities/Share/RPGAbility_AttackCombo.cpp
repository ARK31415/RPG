// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Share/RPGAbility_AttackCombo.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbility_AttackCombo, All, All)

URPGAbility_AttackCombo::URPGAbility_AttackCombo()
	: MaxComboCount(1)
	, ComboWindowTime(0.5f)
	, CurrentComboIndex(0)
	, bPendingComboInput(false)
{
}

void URPGAbility_AttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] ActivateAbility - MaxComboCount: %d"), *GetName(), MaxComboCount);

	ResetComboState();

	if (ComboMontages.IsEmpty())
	{
		UE_LOG(LogRPGAbility_AttackCombo, Error, TEXT("[%s] ActivateAbility - ComboMontages is empty!"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 监听连招窗口期事件
	WaitForComboWindowEvents();

	// 播放第一段连招
	PlayCurrentComboMontage();
}

void URPGAbility_AttackCombo::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] EndAbility - ComboIndex: %d, WasCancelled: %s"),
		*GetName(), CurrentComboIndex, bWasCancelled ? TEXT("true") : TEXT("false"));

	ResetComboState();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ========== 连招核心逻辑 ==========

void URPGAbility_AttackCombo::PlayCurrentComboMontage()
{
	UAnimMontage* MontageToPlay = GetCurrentComboMontage();
	if (!MontageToPlay)
	{
		UE_LOG(LogRPGAbility_AttackCombo, Error, TEXT("[%s] PlayCurrentComboMontage - Montage is null at ComboIndex: %d"), *GetName(), CurrentComboIndex);
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] PlayCurrentComboMontage - ComboIndex: %d, Montage: %s"),
		*GetName(), CurrentComboIndex, *MontageToPlay->GetName());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	if (!MontageTask)
	{
		UE_LOG(LogRPGAbility_AttackCombo, Error, TEXT("[%s] PlayCurrentComboMontage - MontageTask creation failed!"), *GetName());
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	CurrentMontageTask = MontageTask;

	MontageTask->OnCompleted.AddDynamic(this, &URPGAbility_AttackCombo::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGAbility_AttackCombo::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGAbility_AttackCombo::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGAbility_AttackCombo::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] IsPlayingMontage: true"), *GetName());
}

void URPGAbility_AttackCombo::OnComboInputReceived()
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnComboInputReceived - CurrentComboIndex: %d, CanContinue: %s"),
		*GetName(), CurrentComboIndex, CanContinueCombo() ? TEXT("true") : TEXT("false"));

	if (CanContinueCombo())
	{
		bPendingComboInput = true;
	}
}

bool URPGAbility_AttackCombo::CanContinueCombo() const
{
	return CurrentComboIndex < MaxComboCount - 1
		&& CurrentComboIndex < ComboMontages.Num() - 1;
}

void URPGAbility_AttackCombo::AdvanceCombo()
{
	CurrentComboIndex++;
	bPendingComboInput = false;

	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] AdvanceCombo - New ComboIndex: %d"), *GetName(), CurrentComboIndex);

	PlayCurrentComboMontage();
}

void URPGAbility_AttackCombo::ResetComboState()
{
	CurrentComboIndex = 0;
	bPendingComboInput = false;
	CurrentMontageTask = nullptr;
}

UAnimMontage* URPGAbility_AttackCombo::GetCurrentComboMontage() const
{
	if (ComboMontages.IsValidIndex(CurrentComboIndex))
	{
		return ComboMontages[CurrentComboIndex];
	}
	return nullptr;
}

// ========== Montage回调 ==========

void URPGAbility_AttackCombo::OnMontageCompleted()
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnMontageCompleted - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void URPGAbility_AttackCombo::OnMontageBlendOut()
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnMontageBlendOut - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void URPGAbility_AttackCombo::OnMontageInterrupted()
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnMontageInterrupted - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void URPGAbility_AttackCombo::OnMontageCancelled()
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnMontageCancelled - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

// ========== 连招窗口期 ==========

void URPGAbility_AttackCombo::WaitForComboWindowEvents()
{
	// 监听窗口期开启事件
	UAbilityTask_WaitGameplayEvent* WaitOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_ComboWindow_Open,
		nullptr,
		false,
		true
	);
	WaitOpenTask->EventReceived.AddDynamic(this, &URPGAbility_AttackCombo::OnComboWindowOpen);
	WaitOpenTask->ReadyForActivation();

	// 监听窗口期关闭事件
	UAbilityTask_WaitGameplayEvent* WaitCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_ComboWindow_Close,
		nullptr,
		false,
		true
	);
	WaitCloseTask->EventReceived.AddDynamic(this, &URPGAbility_AttackCombo::OnComboWindowClose);
	WaitCloseTask->ReadyForActivation();
}

void URPGAbility_AttackCombo::OnComboWindowOpen(FGameplayEventData Payload)
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnComboWindowOpen - ComboIndex: %d"), *GetName(), CurrentComboIndex);

	// 窗口期开启：如果已有pending输入则立即推进
	if (bPendingComboInput && CanContinueCombo())
	{
		AdvanceCombo();
	}
}

void URPGAbility_AttackCombo::OnComboWindowClose(FGameplayEventData Payload)
{
	UE_LOG(LogRPGAbility_AttackCombo, Log, TEXT("[%s] OnComboWindowClose - ComboIndex: %d, PendingInput: %s"),
		*GetName(), CurrentComboIndex, bPendingComboInput ? TEXT("true") : TEXT("false"));

	// 窗口期关闭：如果有pending输入则推进，否则等Montage自然结束
	if (bPendingComboInput && CanContinueCombo())
	{
		AdvanceCombo();
	}
}
