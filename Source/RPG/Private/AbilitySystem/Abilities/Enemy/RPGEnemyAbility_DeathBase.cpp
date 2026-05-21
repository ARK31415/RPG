// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_DeathBase.h"
#include "Character/RPGEnemyCharacter.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "RPGDebugHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyDeath, Log, All)

URPGEnemyAbility_DeathBase::URPGEnemyAbility_DeathBase()
{
}

ARPGEnemyCharacter* URPGEnemyAbility_DeathBase::GetEnemyCharacterFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGEnemyCharacter>(ActorInfo->AvatarActor.Get());
	}
	return nullptr;
}

UEnemyCombatComponent* URPGEnemyAbility_DeathBase::GetEnemyCombatComponentFromActorInfo() const
{
	if (ARPGEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo())
	{
		return EnemyCharacter->FindComponentByClass<UEnemyCombatComponent>();
	}
	return nullptr;
}

void URPGEnemyAbility_DeathBase::StartDeathSequence_Implementation(AActor* AvatarActor)
{
	// 调用父类（会触发 IPawnDeathInterface::OnDeathStarted）
	Super::StartDeathSequence_Implementation(AvatarActor);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyDeath, Log, TEXT("Enemy DeathStarted: %s"), *EnemyCharacter->GetName());
	Debug::Print(FString::Printf(TEXT("[Death] EnemyAbility::StartDeathSequence - %s"), *EnemyCharacter->GetName()));

	// 调用蓝图可重写钩子（用于特效、掉落物等）
	OnEnemyDeathStarted(EnemyCharacter);
}

void URPGEnemyAbility_DeathBase::FinishDeathSequence_Implementation(AActor* AvatarActor)
{
	// 调用父类（会触发 IPawnDeathInterface::OnDeathFinished）
	Super::FinishDeathSequence_Implementation(AvatarActor);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyDeath, Log, TEXT("Enemy DeathFinished: %s"), *EnemyCharacter->GetName());
	Debug::Print(FString::Printf(TEXT("[Death] EnemyAbility::FinishDeathSequence - %s"), *EnemyCharacter->GetName()));

	// 调用蓝图可重写钩子（用于经验值、任务更新等）
	OnEnemyDeathFinished(EnemyCharacter);
}

void URPGEnemyAbility_DeathBase::OnEnemyDeathStarted_Implementation(ARPGEnemyCharacter* EnemyCharacter)
{
	// 默认实现为空，蓝图子类可重写以添加特效、掉落物等
}

void URPGEnemyAbility_DeathBase::OnEnemyDeathFinished_Implementation(ARPGEnemyCharacter* EnemyCharacter)
{
	// 默认实现为空，蓝图子类可重写以触发经验值奖励、任务更新等
}
