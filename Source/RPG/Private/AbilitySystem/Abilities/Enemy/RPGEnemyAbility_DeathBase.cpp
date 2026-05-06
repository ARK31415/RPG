// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Enemy/RPGEnemyAbility_DeathBase.h"
#include "Character/RPGEnemyCharacter.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyDeath, Log, All)

URPGEnemyAbility_DeathBase::URPGEnemyAbility_DeathBase()
{
	EnemyDestroyDelay = 5.0f;
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
	// 先执行共享死亡逻辑（禁用碰撞、停止移动、通知接口）
	Super::StartDeathSequence_Implementation(AvatarActor);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyDeath, Log, TEXT("Enemy DeathStarted: %s"), *EnemyCharacter->GetName());

	// 停止 AI 行为树
	if (AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Death"));
		}
	}

	// 调用蓝图可重写钩子
	OnEnemyDeathStarted(EnemyCharacter);
}

void URPGEnemyAbility_DeathBase::FinishDeathSequence_Implementation(AActor* AvatarActor)
{
	// 先执行共享死亡完成逻辑
	Super::FinishDeathSequence_Implementation(AvatarActor);

	ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(AvatarActor);
	if (!EnemyCharacter)
	{
		return;
	}

	UE_LOG(LogRPGEnemyDeath, Log, TEXT("Enemy DeathFinished: %s"), *EnemyCharacter->GetName());

	// 设置延迟销毁
	EnemyCharacter->SetLifeSpan(EnemyDestroyDelay);

	// 调用蓝图可重写钩子
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
