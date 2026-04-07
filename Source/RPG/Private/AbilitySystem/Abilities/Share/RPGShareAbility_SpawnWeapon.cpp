// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Share/RPGShareAbility_SpawnWeapon.h"

#include "Component/Combat/PawnCombatComponent.h"
#include "Items/Weapon/RPGWeaponBase.h"

void URPGShareAbility_SpawnWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !WeaponClassToSpawn)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// 1. 生成武器Actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedWeapon = GetWorld()->SpawnActor<ARPGWeaponBase>(WeaponClassToSpawn, FTransform::Identity, SpawnParams);
	if (!SpawnedWeapon)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// 2. 附加到角色骨骼的指定Socket
	if (USkeletalMeshComponent* OwnerMesh = AvatarActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		SpawnedWeapon->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketNameToAttach);
	}

	// 3. 登记到CombatComponent
	if (UPawnCombatComponent* CombatComp = GetPawnCombatComponentFromActorInfo())
	{
		CombatComp->RegisterSpawnWeapon(WeaponTagToRegister, SpawnedWeapon, bRegisterAsEquippedWeapon);
	}
}

void URPGShareAbility_SpawnWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SpawnedWeapon)
	{
		SpawnedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SpawnedWeapon->Destroy();
		SpawnedWeapon = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
