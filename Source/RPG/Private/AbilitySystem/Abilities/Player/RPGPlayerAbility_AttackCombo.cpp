#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_AttackCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Character/RPGPlayerCharacter.h"
#include "RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "GameplayEffect.h"

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
		// 切换攻击类型：重置对方计数器
		CombatComp->SwitchComboType(ComboType);
		CurrentLightAttackComboCount = CombatComp->GetComboCount(ComboType);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Synced combo count from CombatComponent (type %d): %d"), static_cast<uint8>(ComboType), CurrentLightAttackComboCount);
	}

	// 设置当前攻击类型 (用于伤害计算)
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CombatComp->SetCurrentComboType(ComboType);
	}

	// 监听 MeleeHit 事件 (Warrior 模式)
	WaitMeleeHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_MeleeHit,
		nullptr,
		false,
		true
	);

	if (WaitMeleeHitEventTask)
	{
		WaitMeleeHitEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::HandleApplyDamage);
		WaitMeleeHitEventTask->ReadyForActivation();
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] WaitMeleeHitEventTask activated"));
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
		CombatComp->SetComboCount(ComboType, CurrentLightAttackComboCount);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Synced combo count to CombatComponent (type %d): %d"), static_cast<uint8>(ComboType), CurrentLightAttackComboCount);
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
		CombatComp->AdvanceComboCount(ComboType, MaxComboCount);
		CurrentLightAttackComboCount = CombatComp->GetComboCount(ComboType);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Combo count advanced via CombatComponent (type %d): %d"), static_cast<uint8>(ComboType), CurrentLightAttackComboCount);
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
		CombatComp->ResetComboCount(ComboType);
		CurrentLightAttackComboCount = CombatComp->GetComboCount(ComboType);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] Combo count reset via CombatComponent (type %d) to: %d"), static_cast<uint8>(ComboType), CurrentLightAttackComboCount);
	}
}

void URPGPlayerAbility_AttackCombo::StartComboWindowTimer()
{
	// 委托给 CombatComponent 处理定时器
	if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
	{
		CombatComp->StartComboWindowTimer(ComboType, ComboWindowTime);
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Combo window timer started via CombatComponent (type %d, %.2f seconds)"), static_cast<uint8>(ComboType), ComboWindowTime);
	}
}

void URPGPlayerAbility_AttackCombo::HandleApplyDamage(FGameplayEventData EventData)
{
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] ===== HandleApplyDamage ====="));

	FGameplayEventData LocalEventData = EventData;
	// 1. 获取目标 (从 EventData)
	AActor* TargetActor = const_cast<AActor*>(LocalEventData.Target.Get());
	if (!TargetActor)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: TargetActor is null!"));
		return;
	}
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Target: %s"), *TargetActor->GetName());

	// 2. 获取伤害数据 (从 CombatComponent)
	UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo();
	if (!CombatComp)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: CombatComponent is null!"));
		return;
	}

	// 获取Ability等级
	UAbilitySystemComponent* OwningASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwningASC)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: ASC is null!"));
		return;
	}

	float AbilityLevel = GetAbilityLevel();
	float BaseDamage = CombatComp->GetPlayerCurrentEquippedWeaponDamageAtLevel(AbilityLevel);
	int32 ComboCount = CombatComp->GetComboCount(ComboType);

	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] BaseDamage: %.2f, ComboCount: %d, AbilityLevel: %.0f"),
		BaseDamage, ComboCount, AbilityLevel);

	// 3. 创建 GE Spec (使用辅助函数)
	if (!DamageEffectClass)
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: DamageEffectClass is not set!"));
		return;
	}

	// 确定当前攻击类型的 Tag
	FGameplayTag AttackTypeTag;
	if (ComboType == ERPGComboType::LightAttack)
	{
		AttackTypeTag = RPGGameplayTags::Player_SetByCaller_AttackType_Light;
	}
	else if (ComboType == ERPGComboType::HeavyAttack)
	{
		AttackTypeTag = RPGGameplayTags::Player_SetByCaller_AttackType_Heavy;
	}

	// 使用基类辅助函数创建 Spec
	FGameplayEffectSpecHandle SpecHandle = MakePlayerDamageEffectSpecHandle(
		DamageEffectClass,
		BaseDamage,
		AttackTypeTag,
		ComboCount
	);

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogRPGPlayerAbility_AttackCombo, Error, TEXT("[PlayerAttackCombo] ERROR: Failed to create SpecHandle!"));
		return;
	}

	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Created Damage Spec with BaseDamage: %.2f, ComboCount: %d, AttackType: %s"),
		BaseDamage, ComboCount, *AttackTypeTag.ToString());

	// 4. 应用 GE 到目标 (使用基类辅助函数)
	NativeApplyEffectSpecHandleToTarget(TargetActor, SpecHandle);
	UE_LOG(LogRPGPlayerAbility_AttackCombo, Warning, TEXT("[PlayerAttackCombo] Applied Damage GE to Target: %s"), *TargetActor->GetName());

	// 5. 发送 HitReact 事件给目标 (Warrior 模式)
	SendHitReactEvent(TargetActor, LocalEventData);
}

void URPGPlayerAbility_AttackCombo::SendHitReactEvent(AActor* TargetActor, FGameplayEventData EventData)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		TargetActor,
		RPGGameplayTags::Shared_Event_HitReact,
		EventData
	);

	UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, TEXT("[PlayerAttackCombo] Sent HitReact event to Target: %s"), *TargetActor->GetName());
}
