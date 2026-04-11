// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Combat/EnemyCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyCombatComponent, All, All)

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	if (OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);

	// TODO: 敌人命中玩家时的逻辑
	// - 发送MeleeHit事件
	// - 触发伤害计算

	UE_LOG(LogEnemyCombatComponent, Log, TEXT("EnemyCombatComponent::OnHitTargetActor - Hit: %s"), *HitActor->GetName());

	FGameplayEventData Data;
	Data.Instigator = GetOwningPawn();
	Data.Target = HitActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Shared_Event_MeleeHit,
		Data
	);
}

void UEnemyCombatComponent::OnWeaponPullerFromTargetActor(AActor* InteractedActor)
{
	UE_LOG(LogEnemyCombatComponent, Log, TEXT("EnemyCombatComponent::OnWeaponPullerFromTargetActor - Actor: %s"),
		InteractedActor ? *InteractedActor->GetName() : TEXT("null"));
}
