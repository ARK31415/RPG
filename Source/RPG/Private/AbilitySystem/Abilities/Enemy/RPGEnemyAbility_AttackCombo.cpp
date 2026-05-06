#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_AttackCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "Character/RPGEnemyCharacter.h"
#include "RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

URPGEnemyAbility_AttackCombo::URPGEnemyAbility_AttackCombo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void URPGEnemyAbility_AttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Activated: %s"), *GetName());

	ResetComboState();
	PlayCurrentComboMontage();
}

void URPGEnemyAbility_AttackCombo::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

void URPGEnemyAbility_AttackCombo::AdvanceCombo()
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

bool URPGEnemyAbility_AttackCombo::CanContinueCombo() const
{
	return CurrentComboIndex < MaxComboCount - 1;
}

void URPGEnemyAbility_AttackCombo::ResetComboState()
{
	CurrentComboIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Reset combo state"));
}

void URPGEnemyAbility_AttackCombo::PlayCurrentComboMontage()
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

	MontageTask->OnCompleted.AddDynamic(this, &URPGEnemyAbility_AttackCombo::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGEnemyAbility_AttackCombo::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGEnemyAbility_AttackCombo::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGEnemyAbility_AttackCombo::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

UEnemyCombatComponent* URPGEnemyAbility_AttackCombo::GetCombatComponentFromActorInfo() const
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

void URPGEnemyAbility_AttackCombo::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Montage Completed - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGEnemyAbility_AttackCombo::OnMontageBlendOut()
{
	UE_LOG(LogTemp, Log, TEXT("[EnemyAttackCombo] Montage BlendOut - Index: %d"), CurrentComboIndex);
}

void URPGEnemyAbility_AttackCombo::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttackCombo] Montage Interrupted - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGEnemyAbility_AttackCombo::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttackCombo] Montage Cancelled - Index: %d"), CurrentComboIndex);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
