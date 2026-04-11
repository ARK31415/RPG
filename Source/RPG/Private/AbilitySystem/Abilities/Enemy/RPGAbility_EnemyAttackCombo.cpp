#include "AbilitySystem/Abilities/Enemy/RPGAbility_EnemyAttackCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "Character/RPGEnemyCharacter.h"
#include "RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

URPGAbility_EnemyAttackCombo::URPGAbility_EnemyAttackCombo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void URPGAbility_EnemyAttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Activated: %s"), *GetName());

	ResetComboState();
	PlayCurrentComboMontage();
}

void URPGAbility_EnemyAttackCombo::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(ActorInfo->AvatarActor.Get()))
		{
			CachedCombatComponent = EnemyCharacter->GetEnemyCombatComponent();
		}
	}
}

void URPGAbility_EnemyAttackCombo::AdvanceCombo()
{
	if (!CanContinueCombo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAttackCombo] Cannot advance combo - Max reached: %d"), CurrentComboIndex);
		return;
	}

	CurrentComboIndex++;

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Advanced to combo index: %d"), CurrentComboIndex);

	PlayCurrentComboMontage();
}

bool URPGAbility_EnemyAttackCombo::CanContinueCombo() const
{
	return CurrentComboIndex < MaxComboCount - 1;
}

void URPGAbility_EnemyAttackCombo::ResetComboState()
{
	CurrentComboIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Reset combo state"));
}

void URPGAbility_EnemyAttackCombo::PlayCurrentComboMontage()
{
	if (ComboMontages.Num() == 0 || CurrentComboIndex >= ComboMontages.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAttackCombo] No valid montage for index %d (Total: %d)"), 
			CurrentComboIndex, ComboMontages.Num());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = ComboMontages[CurrentComboIndex];
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAttackCombo] Montage at index %d is null"), CurrentComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Playing montage: %s (Index: %d)"), *MontageToPlay->GetName(), CurrentComboIndex);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		FName(),
		0.0f,
		true
	);

	MontageTask->OnCompleted.AddDynamic(this, &URPGAbility_EnemyAttackCombo::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGAbility_EnemyAttackCombo::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGAbility_EnemyAttackCombo::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGAbility_EnemyAttackCombo::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

UEnemyCombatComponent* URPGAbility_EnemyAttackCombo::GetCombatComponentFromActorInfo() const
{
	if (CachedCombatComponent.IsValid())
	{
		return CachedCombatComponent.Get();
	}

	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			return EnemyCharacter->GetEnemyCombatComponent();
		}
	}

	return nullptr;
}

void URPGAbility_EnemyAttackCombo::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Montage Completed - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGAbility_EnemyAttackCombo::OnMontageBlendOut()
{
	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Montage BlendOut - Index: %d"), CurrentComboIndex);
}

void URPGAbility_EnemyAttackCombo::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttackCombo] Montage Interrupted - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGAbility_EnemyAttackCombo::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttackCombo] Montage Cancelled - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
