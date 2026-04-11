// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Share/RPGAbility_MeleeAttackCombo.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Component/Combat/PawnCombatComponent.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAbility_MeleeAttackCombo, All, All)

void URPGAbility_MeleeAttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogRPGAbility_MeleeAttackCombo, Log, TEXT("[%s] ActivateAbility - MeleeAttackCombo"), *GetName());

	// 监听武器碰撞开关事件
	WaitForWeaponCollisionEvents();

	// 调用父类（启动连招逻辑）
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UPawnCombatComponent* URPGAbility_MeleeAttackCombo::GetCombatComponent() const
{
	return GetPawnCombatComponentFromActorInfo();
}

void URPGAbility_MeleeAttackCombo::OnWeaponCollisionEnable(FGameplayEventData Payload)
{
	UPawnCombatComponent* CombatComp = GetCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogRPGAbility_MeleeAttackCombo, Error, TEXT("[%s] OnWeaponCollisionEnable - CombatComponent is null!"), *GetName());
		return;
	}

	UE_LOG(LogRPGAbility_MeleeAttackCombo, Log, TEXT("[%s] OnWeaponCollisionEnable - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	CombatComp->ToggleWeaponCollision(true, EToggleDamageType::CurrentEquippedWeapon);
}

void URPGAbility_MeleeAttackCombo::OnWeaponCollisionDisable(FGameplayEventData Payload)
{
	UPawnCombatComponent* CombatComp = GetCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogRPGAbility_MeleeAttackCombo, Error, TEXT("[%s] OnWeaponCollisionDisable - CombatComponent is null!"), *GetName());
		return;
	}

	UE_LOG(LogRPGAbility_MeleeAttackCombo, Log, TEXT("[%s] OnWeaponCollisionDisable - ComboIndex: %d"), *GetName(), CurrentComboIndex);
	CombatComp->ToggleWeaponCollision(false, EToggleDamageType::CurrentEquippedWeapon);
}

void URPGAbility_MeleeAttackCombo::WaitForWeaponCollisionEvents()
{
	// 监听武器碰撞开启事件
	UAbilityTask_WaitGameplayEvent* WaitEnableTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_Melee_CollisionEnable,
		nullptr,
		false,
		true
	);
	WaitEnableTask->EventReceived.AddDynamic(this, &URPGAbility_MeleeAttackCombo::OnWeaponCollisionEnable);
	WaitEnableTask->ReadyForActivation();

	// 监听武器碰撞关闭事件
	UAbilityTask_WaitGameplayEvent* WaitDisableTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Shared_Event_Melee_CollisionDisable,
		nullptr,
		false,
		true
	);
	WaitDisableTask->EventReceived.AddDynamic(this, &URPGAbility_MeleeAttackCombo::OnWeaponCollisionDisable);
	WaitDisableTask->ReadyForActivation();
}
