#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_AttackCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Character/RPGPlayerCharacter.h"
#include "RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

URPGPlayerAbility_AttackCombo::URPGPlayerAbility_AttackCombo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void URPGPlayerAbility_AttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Activated: %s"), *GetName());

	ResetComboState();
	WaitForComboWindowEvents();
	PlayCurrentComboMontage();
}

void URPGPlayerAbility_AttackCombo::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			CachedCombatComponent = PlayerCharacter->GetPlayerCombatComponent();
		}
	}
}

void URPGPlayerAbility_AttackCombo::OnComboInputReceived()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] OnComboInputReceived - CurrentComboIndex: %d, bIsInComboWindow: %d"), 
		CurrentComboIndex, bIsInComboWindow);

	if (bIsInComboWindow && CanContinueCombo())
	{
		bPendingComboInput = true;
		UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Combo input queued - Index: %d"), CurrentComboIndex);
	}
}

bool URPGPlayerAbility_AttackCombo::CanContinueCombo() const
{
	return CurrentComboIndex < MaxComboCount - 1;
}

void URPGPlayerAbility_AttackCombo::AdvanceCombo()
{
	if (!CanContinueCombo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerAttackCombo] Cannot advance combo - Max reached: %d"), CurrentComboIndex);
		return;
	}

	CurrentComboIndex++;
	bPendingComboInput = false;
	bIsInComboWindow = false;

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Advanced to combo index: %d"), CurrentComboIndex);

	PlayCurrentComboMontage();
}

void URPGPlayerAbility_AttackCombo::ResetComboState()
{
	CurrentComboIndex = 0;
	bPendingComboInput = false;
	bIsInComboWindow = false;

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Reset combo state"));
}

void URPGPlayerAbility_AttackCombo::PlayCurrentComboMontage()
{
	if (ComboMontages.Num() == 0 || CurrentComboIndex >= ComboMontages.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerAttackCombo] No valid montage for index %d (Total: %d)"), 
			CurrentComboIndex, ComboMontages.Num());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = ComboMontages[CurrentComboIndex];
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerAttackCombo] Montage at index %d is null"), CurrentComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Playing montage: %s (Index: %d)"), *MontageToPlay->GetName(), CurrentComboIndex);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		FName(),
		0.0f,
		true
	);

	MontageTask->OnCompleted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

UPlayerCombatComponent* URPGPlayerAbility_AttackCombo::GetCombatComponentFromActorInfo() const
{
	if (CachedCombatComponent.IsValid())
	{
		return CachedCombatComponent.Get();
	}

	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			return PlayerCharacter->GetPlayerCombatComponent();
		}
	}

	return nullptr;
}

void URPGPlayerAbility_AttackCombo::OnComboWindowOpen(FGameplayEventData Payload)
{
	bIsInComboWindow = true;

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Combo window OPEN - Index: %d, bPendingComboInput: %d"), 
		CurrentComboIndex, bPendingComboInput);

	if (bPendingComboInput && CanContinueCombo())
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Executing pending combo - Index: %d"), CurrentComboIndex);
		AdvanceCombo();
	}
}

void URPGPlayerAbility_AttackCombo::OnComboWindowClose(FGameplayEventData Payload)
{
	bIsInComboWindow = false;
	bPendingComboInput = false;

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Combo window CLOSED - Index: %d"), CurrentComboIndex);
}

void URPGPlayerAbility_AttackCombo::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Montage Completed - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGPlayerAbility_AttackCombo::OnMontageBlendOut()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Montage BlendOut - Index: %d"), CurrentComboIndex);
}

void URPGPlayerAbility_AttackCombo::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayerAttackCombo] Montage Interrupted - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGPlayerAbility_AttackCombo::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayerAttackCombo] Montage Cancelled - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGPlayerAbility_AttackCombo::WaitForComboWindowEvents()
{
	// Wait for ComboWindow.Open
	UAbilityTask_WaitGameplayEvent* OpenEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_ComboWindow_Open,
		nullptr,
		false,
		true
	);
	OpenEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnComboWindowOpen);
	OpenEventTask->ReadyForActivation();

	// Wait for ComboWindow.Close
	UAbilityTask_WaitGameplayEvent* CloseEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_ComboWindow_Close,
		nullptr,
		false,
		true
	);
	CloseEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnComboWindowClose);
	CloseEventTask->ReadyForActivation();

	UE_LOG(LogTemp, Log, TEXT("[PlayerAttackCombo] Waiting for combo window events"));
}
