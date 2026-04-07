// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_UnequipSword.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimationInstances/RPGCharacterAnimInstance.h"
#include "Character/RPGPlayerCharacter.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Controllers/RPGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "RPGGameplayTags.h"

void URPGPlayerAbility_UnequipSword::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CachedPlayerCharacter = GetPlayerCharacterFromActorInfo();
	CachedPlayerController = GetPlayerControllerFromActorInfo();
	CombatComponent = CachedPlayerCharacter->GetPlayerCombatComponent();

	CachedEquippedWeapon = GetCurrentEquippedWeapon();
	if (!CachedEquippedWeapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 步骤1: 播放卸下动画蒙太奇
	const FRPGPlayerWeaponData& WeaponData = CachedEquippedWeapon->PlayerWeaponData;
	if (!WeaponData.UnequipWeaponMontage)
	{
		// 如果没有配置卸下动画，直接执行卸下逻辑
		DetachWeaponFromCharacter();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		WeaponData.UnequipWeaponMontage,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	MontageTask->OnCompleted.AddDynamic(this, &URPGPlayerAbility_UnequipSword::OnUnequipMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGPlayerAbility_UnequipSword::OnUnequipMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGPlayerAbility_UnequipSword::OnUnequipMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGPlayerAbility_UnequipSword::OnUnequipMontageCancelled);
	MontageTask->ReadyForActivation();
}

void URPGPlayerAbility_UnequipSword::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

ARPGPlayerWeapon* URPGPlayerAbility_UnequipSword::GetCurrentEquippedWeapon() const
{
	return CombatComponent->GetPlayerCurrentEquippedWeapon();
}

void URPGPlayerAbility_UnequipSword::UnlinkWeaponAnimLayer()
{
	if (!CachedPlayerCharacter) return;

	if (USkeletalMeshComponent* MeshComp = CachedPlayerCharacter->GetMesh())
	{
		if (URPGCharacterAnimInstance* AnimInstance = Cast<URPGCharacterAnimInstance>(MeshComp->GetAnimInstance()))
		{
			AnimInstance->UnlinkAnimLayer();
		}
	}
}

void URPGPlayerAbility_UnequipSword::RemoveWeaponInputMappingContext(ARPGPlayerWeapon* Weapon)
{
	if (!Weapon) return;

	UInputMappingContext* WeaponIMC = Weapon->PlayerWeaponData.WeaponInputMappingContext;
	if (!WeaponIMC) return;

	ULocalPlayer* LocalPlayer = CachedPlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (InputSubsystem->HasMappingContext(WeaponIMC))
		{
			InputSubsystem->RemoveMappingContext(WeaponIMC);
		}
	}
}

void URPGPlayerAbility_UnequipSword::RemoveWeaponAbilities(ARPGPlayerWeapon* Weapon)
{
	if (!Weapon) return;

	TArray<FGameplayAbilitySpecHandle> GrantedHandles = Weapon->GetGrantAbilitySpecHandles();
	if (GrantedHandles.IsEmpty()) return;

	URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	ASC->RemovedGrantPlayerWeaponAbility(GrantedHandles);
	Weapon->AssignGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>());
}

void URPGPlayerAbility_UnequipSword::OnUnequipMontageCompleted()
{
	// 动画播放完成，等待GameplayEvent
	WaitForUnequipGameplayEvent();
}

void URPGPlayerAbility_UnequipSword::OnUnequipMontageBlendOut()
{
	// 动画混合退出，等待GameplayEvent
	WaitForUnequipGameplayEvent();
}

void URPGPlayerAbility_UnequipSword::OnUnequipMontageInterrupted()
{
	// 动画被中断，结束能力
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void URPGPlayerAbility_UnequipSword::OnUnequipMontageCancelled()
{
	// 动画被取消，结束能力
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void URPGPlayerAbility_UnequipSword::WaitForUnequipGameplayEvent()
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Player_Event_Unequip_Sword,
		nullptr,
		false,
		true
	);

	WaitEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_UnequipSword::OnUnequipGameplayEventReceived);
	WaitEventTask->ReadyForActivation();
}

void URPGPlayerAbility_UnequipSword::OnUnequipGameplayEventReceived(FGameplayEventData Payload)
{
	// 接收到UnequipSword事件，执行分离武器
	DetachWeaponFromCharacter();
}

void URPGPlayerAbility_UnequipSword::DetachWeaponFromCharacter()
{
	if (!CachedEquippedWeapon || !CachedPlayerCharacter)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	CombatComponent->CurrentEquippedWeaponTag = FGameplayTag::EmptyTag;

	// 先执行卸下逻辑（与Equip相反的顺序）
	RemoveWeaponAbilities(CachedEquippedWeapon);
	RemoveWeaponInputMappingContext(CachedEquippedWeapon);
	UnlinkWeaponAnimLayer();

	// 将武器从角色上分离
	CachedEquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 正常结束能力
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
