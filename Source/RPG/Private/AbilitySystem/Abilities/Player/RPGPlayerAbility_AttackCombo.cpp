#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_AttackCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Character/RPGPlayerCharacter.h"
#include "RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerAbility_AttackCombo, All, All)

URPGPlayerAbility_AttackCombo::URPGPlayerAbility_AttackCombo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void URPGPlayerAbility_AttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	
	// 动态设置 MaxComboCount（因为构造函数中 ComboMontages 还是空的）
	if (MaxComboCount <= 0)
	{
		MaxComboCount = ComboMontages.Num();
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] MaxComboCount dynamically set to: %d"), MaxComboCount);
	}
	
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ===== ActivateAbility ====="));

	// 激活时从 CombatComponent 获取当前连招计数
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CurrentLightAttackComboCount = CombatComp->GetCurrentComboCount();
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Synced combo count from CombatComponent: %d"), CurrentLightAttackComboCount);
	}

	PlayCurrentComboMontage();
}

void URPGPlayerAbility_AttackCombo::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilitySpec& Spec)
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

void URPGPlayerAbility_AttackCombo::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               bool bReplicateEndAbility, bool bWasCancelled)
{
	AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] ===== EndAbility ====="));
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] Current Combo Count: %d"), CurrentLightAttackComboCount);
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] bWasCancelled: %s"), bWasCancelled ? TEXT("true") : TEXT("false"));
	
	// 能力结束时，将连招计数同步到 CombatComponent，并启动定时器
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CombatComp->SetCurrentComboCount(CurrentLightAttackComboCount);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Synced combo count to CombatComponent: %d"), CurrentLightAttackComboCount);
	}
	
	// 启动连招窗口定时器
	StartComboWindowTimer();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URPGPlayerAbility_AttackCombo::PlayCurrentComboMontage()
{
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] ----- PlayCurrentComboMontage -----"));
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] Requested combo index: %d"), CurrentLightAttackComboCount);
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] ComboMontages map size: %d"), ComboMontages.Num());
	
	// Validate that the combo index exists in the map
	if (!ComboMontages.Contains(CurrentLightAttackComboCount))
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: No montage mapped for combo index %d (Map Size: %d)"),
		       CurrentLightAttackComboCount, ComboMontages.Num());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = ComboMontages[CurrentLightAttackComboCount];
	
	if (!MontageToPlay)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: Montage at index %d is null!"), CurrentLightAttackComboCount);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] Found montage: %s"), *MontageToPlay->GetName());
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] Montage duration: %.2f seconds"), MontageToPlay->GetPlayLength());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		FName(),
		0.0f,
		true
	);

	if (!MontageTask)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: Failed to create MontageTask!"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] MontageTask created successfully"));

	AdvanceComboCount();

	MontageTask->OnCompleted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCancelled);
	MontageTask->ReadyForActivation();
	
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] MontageTask activated and playing"));
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

void URPGPlayerAbility_AttackCombo::AdvanceComboCount()
{
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] >>> AdvanceComboCount called <<<"));
	
	if (!GetWorld())
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: Cannot advance combo - GetWorld() is null"));
		return;
	}

	// 使用 CombatComponent 管理连招计数
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CombatComp->AdvanceComboCount(MaxComboCount);
		CurrentLightAttackComboCount = CombatComp->GetCurrentComboCount();
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Combo count advanced via CombatComponent: %d"), CurrentLightAttackComboCount);
	}
	else
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: CombatComponent is null!"));
	}
}


void URPGPlayerAbility_AttackCombo::OnMontageCompleted()
{
	/*AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] ===== OnMontageCompleted ====="));
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Avatar Actor: %s"), AvatarActor ? *AvatarActor->GetName() : TEXT("None"));
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Final combo index played: %d"), CurrentLightAttackComboCount);
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Ending ability (success)"));*/
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGPlayerAbility_AttackCombo::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGPlayerAbility_AttackCombo::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGPlayerAbility_AttackCombo::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URPGPlayerAbility_AttackCombo::ResetComboCount()
{
	// 委托给 CombatComponent 处理
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CombatComp->ResetComboCount();
		CurrentLightAttackComboCount = CombatComp->GetCurrentComboCount();
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] Combo count reset via CombatComponent to: %d"), CurrentLightAttackComboCount);
	}
}

void URPGPlayerAbility_AttackCombo::StartComboWindowTimer()
{
	// 委托给 CombatComponent 处理定时器
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		AActor* Owner = CombatComp->GetOwner();
		if (!Owner)
		{
			UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: CombatComponent owner is null!"));
			return;
		}
		
		UWorld* World = Owner->GetWorld();
		if (!World)
		{
			UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: World is null!"));
			return;
		}
		
		// 清除旧定时器
		World->GetTimerManager().ClearTimer(ComboCountResetTimerHandle);
		
		// 创建委托绑定到 CombatComponent 的定时器回调
		FTimerDynamicDelegate TimerDelegate;
		TimerDelegate.BindUFunction(CombatComp, FName("OnComboWindowTimerExpired"));
		
		World->GetTimerManager().SetTimer(
			ComboCountResetTimerHandle,
			TimerDelegate,
			ComboWindowTime,
			false
		);
		
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Combo window timer started on CombatComponent (%.2f seconds)"), ComboWindowTime);
	}
}
