// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_EquipSword.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimationInstances/RPGCharacterAnimInstance.h"
#include "Animation/AnimationInstances/RPGItemAnimLayersBase.h"
#include "Character/RPGPlayerCharacter.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Controllers/RPGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerAbility_EquipSword, All, All)

void URPGPlayerAbility_EquipSword::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("URPGPlayerAbility_EquipSword::ActivateAbility %s"), *GetName())
	CachedPlayerCharacter = GetPlayerCharacterFromActorInfo();
	CachedPlayerController = GetPlayerControllerFromActorInfo();

	if (!CachedPlayerCharacter || !CachedPlayerController)
	{
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
		       TEXT("URPGPlayerAbility_EquipSword::ActivateAbility - Character or Controller is null!"));
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error, TEXT("  - CachedPlayerCharacter: %s"),
		       CachedPlayerCharacter ? *CachedPlayerCharacter->GetName() : TEXT("None"));
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error, TEXT("  - CachedPlayerController: %s"),
		       CachedPlayerController ? *CachedPlayerController->GetName() : TEXT("None"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	CombatComponent = CachedPlayerCharacter->GetPlayerCombatComponent();
	if (!CombatComponent)
	{
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
		       TEXT("URPGPlayerAbility_EquipSword::ActivateAbility - CombatComponent is null! PlayerCharacter: %s"),
		       *CachedPlayerCharacter->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	CachedSwordWeapon = GetSwordWeapon();
	if (!CachedSwordWeapon)
	{
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
		       TEXT("URPGPlayerAbility_EquipSword::ActivateAbility - SwordWeapon is null!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	const FRPGPlayerWeaponData& WeaponData = CachedSwordWeapon->PlayerWeaponData;
	if (!WeaponData.EquipWeaponMontage)
	{
		// 没有配置装备蒙太奇，直接附加武器并结束
		AttachWeaponToCharacter();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}


	UE_LOG(LogRPGPlayerAbility_EquipSword, Log,
	       TEXT("URPGPlayerAbility_EquipSword::ActivateAbility PlayMontageAndWait %s"), *GetName())
	UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("  - Montage: %s"),
	       WeaponData.EquipWeaponMontage ? *WeaponData.EquipWeaponMontage->GetName() : TEXT("None"));
	UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("  - ActorInfo->AvatarActor (Character): %s"),
	       ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("None"));
	UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("  - ActorInfo->OwnerActor (PlayerState): %s"),
	       ActorInfo && ActorInfo->OwnerActor.IsValid() ? *ActorInfo->OwnerActor->GetName() : TEXT("None"));
	UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("  - Will play on AvatarActor (Character): %s"),
	       ActorInfo && ActorInfo->AvatarActor.IsValid() ? TEXT("YES") : TEXT("NO"));

	// 使用原生 PlayMontageAndWait,ASC 的 ActorInfo 已正确指向 Character(AvatarActor)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		WeaponData.EquipWeaponMontage,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	// 先启动WaitGameplayEvent（捕获蒙太奇播放期间AnimNotify发出的事件）
	WaitForEquipGameplayEvent();

	if (!MontageTask)
	{
		UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
		       TEXT("URPGPlayerAbility_EquipSword::ActivateAbility - MontageTask creation failed! Montage: %s"),
		       WeaponData.EquipWeaponMontage ? *WeaponData.EquipWeaponMontage->GetName() : TEXT("None"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &URPGPlayerAbility_EquipSword::OnEquipMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &URPGPlayerAbility_EquipSword::OnEquipMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &URPGPlayerAbility_EquipSword::OnEquipMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &URPGPlayerAbility_EquipSword::OnEquipMontageCancelled);
	MontageTask->ReadyForActivation();
}

void URPGPlayerAbility_EquipSword::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

ARPGPlayerWeapon* URPGPlayerAbility_EquipSword::GetSwordWeapon() const
{
	return CombatComponent->GetPlayerCarriedWeaponByTag(RPGGameplayTags::Player_Weapon_Sword);
}

void URPGPlayerAbility_EquipSword::LinkWeaponAnimLayer(ARPGPlayerWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	const FRPGPlayerWeaponData& WeaponData = Weapon->PlayerWeaponData;
	if (!WeaponData.WeaponAnimLayerToLink)
	{
		return;
	}

	ARPGPlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	if (!PlayerCharacter)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = PlayerCharacter->GetMesh())
	{
		if (URPGCharacterAnimInstance* AnimInstance = Cast<URPGCharacterAnimInstance>(MeshComp->GetAnimInstance()))
		{
			AnimInstance->LinkAnimLayer(WeaponData.WeaponAnimLayerToLink);
		}
	}
}

void URPGPlayerAbility_EquipSword::AddWeaponInputMappingContext(ARPGPlayerWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	UInputMappingContext* WeaponIMC = Weapon->PlayerWeaponData.WeaponInputMappingContext;
	if (!WeaponIMC)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = CachedPlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (!InputSubsystem->HasMappingContext(WeaponIMC))
		{
			InputSubsystem->AddMappingContext(WeaponIMC, 1);
		}
	}
}

void URPGPlayerAbility_EquipSword::GrantWeaponAbilities(ARPGPlayerWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	const TArray<FRPGPlayerAbilitySet>& WeaponAbilities = Weapon->PlayerWeaponData.DefaultWeaponAbilities;
	if (WeaponAbilities.IsEmpty())
	{
		return;
	}

	URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> GrantedHandles;
	ASC->GrantPlayerWeaponAbility(WeaponAbilities, GetAbilityLevel(), GrantedHandles);
	Weapon->AssignGrantedAbilitySpecHandles(GrantedHandles);
}

void URPGPlayerAbility_EquipSword::OnEquipMontageCompleted()
{
	UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
	       TEXT("URPGPlayerAbility_EquipSword::OnEquipMontageCompleted - Ending Ability from Completed callback"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void URPGPlayerAbility_EquipSword::OnEquipMontageBlendOut()
{
	UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
	       TEXT("URPGPlayerAbility_EquipSword::OnEquipMontageBlendOut - Ending Ability from BlendOut callback"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void URPGPlayerAbility_EquipSword::OnEquipMontageInterrupted()
{
	UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
	       TEXT("URPGPlayerAbility_EquipSword::OnEquipMontageInterrupted - Ending Ability from Interrupted callback"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void URPGPlayerAbility_EquipSword::OnEquipMontageCancelled()
{
	UE_LOG(LogRPGPlayerAbility_EquipSword, Error,
	       TEXT("URPGPlayerAbility_EquipSword::OnEquipMontageCancelled - Ending Ability from Cancelled callback"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void URPGPlayerAbility_EquipSword::WaitForEquipGameplayEvent()
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		RPGGameplayTags::Player_Event_Equip_Sword,
		nullptr,
		false,
		true
	);

	WaitEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_EquipSword::OnEquipGameplayEventReceived);
	WaitEventTask->ReadyForActivation();
}

void URPGPlayerAbility_EquipSword::OnEquipGameplayEventReceived(FGameplayEventData Payload)
{
	// 蒙太奇播放中AnimNotify触发事件 → 附加武器到骨骼插槽
	AttachWeaponToCharacter();
}

void URPGPlayerAbility_EquipSword::AttachWeaponToCharacter()
{
	if (!CombatComponent || !CachedPlayerCharacter)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	// 通过GetPlayerCarriedWeaponByTag从CombatComponent获取已注册的武器
	ARPGPlayerWeapon* SwordWeapon = CombatComponent->GetPlayerCarriedWeaponByTag(RPGGameplayTags::Player_Weapon_Sword);
	if (!SwordWeapon)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	// 将武器附加到角色骨骼的指定Socket
	if (USkeletalMeshComponent* MeshComp = CachedPlayerCharacter->GetMesh())
	{
		SwordWeapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                               SocketNameToAttach);
	}
	else
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}

	// 链接动画层、添加输入映射、赋予武器能力
	LinkWeaponAnimLayer(SwordWeapon);
	AddWeaponInputMappingContext(SwordWeapon);
	GrantWeaponAbilities(SwordWeapon);

	CombatComponent->CurrentEquippedWeaponTag = RPGGameplayTags::Player_Weapon_Sword;
}
