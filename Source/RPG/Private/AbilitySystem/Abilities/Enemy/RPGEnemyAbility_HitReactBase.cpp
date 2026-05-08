// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_HitReactBase.h"
#include "Character/RPGEnemyCharacter.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "AIController.h"
#include "BrainComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyHitReact, Log, All)

ARPGEnemyCharacter* URPGEnemyAbility_HitReactBase::GetEnemyCharacterFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGEnemyCharacter>(ActorInfo->AvatarActor.Get());
	}
	return nullptr;
}

UEnemyCombatComponent* URPGEnemyAbility_HitReactBase::GetEnemyCombatComponentFromActorInfo() const
{
	if (ARPGEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo())
	{
		return EnemyCharacter->FindComponentByClass<UEnemyCombatComponent>();
	}
	return nullptr;
}

void URPGEnemyAbility_HitReactBase::StartHitReactSequence_Implementation(AActor* AvatarActor, FGameplayTag HitDirection)
{
	// 先执行共享受击逻辑（停止移动）
	Super::StartHitReactSequence_Implementation(AvatarActor, HitDirection);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyHitReact, Log, TEXT("Enemy HitReactStarted: %s, Direction: %s"),
		*EnemyCharacter->GetName(), *HitDirection.ToString());

	// 暂停 AI 行为树
	if (AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->PauseLogic(TEXT("HitReact"));
		}
	}

	// 调用蓝图可重写钩子
	OnEnemyHitReactStarted(EnemyCharacter, HitDirection);
}

void URPGEnemyAbility_HitReactBase::FinishHitReactSequence_Implementation(AActor* AvatarActor)
{
	// 先执行共享受击完成逻辑
	Super::FinishHitReactSequence_Implementation(AvatarActor);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyHitReact, Log, TEXT("Enemy HitReactFinished: %s"), *EnemyCharacter->GetName());

	// 恢复 AI 行为树
	if (AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->ResumeLogic(TEXT("HitReact"));
		}
	}

	// 调用蓝图可重写钩子
	OnEnemyHitReactFinished(EnemyCharacter);
}

void URPGEnemyAbility_HitReactBase::OnEnemyHitReactStarted_Implementation(ARPGEnemyCharacter* EnemyCharacter, FGameplayTag HitDirection)
{
	// 默认实现为空，蓝图子类可重写以添加受击特效等
}

void URPGEnemyAbility_HitReactBase::OnEnemyHitReactFinished_Implementation(ARPGEnemyCharacter* EnemyCharacter)
{
	// 默认实现为空，蓝图子类可重写
}
