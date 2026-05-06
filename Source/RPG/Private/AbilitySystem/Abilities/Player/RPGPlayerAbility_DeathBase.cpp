// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_DeathBase.h"
#include "Character/RPGPlayerCharacter.h"
#include "Controllers/RPGPlayerController.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerDeath, Log, All)

ARPGPlayerCharacter* URPGPlayerAbility_DeathBase::GetPlayerCharacterFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGPlayerCharacter>(ActorInfo->AvatarActor.Get());
	}
	return nullptr;
}

ARPGPlayerController* URPGPlayerAbility_DeathBase::GetPlayerControllerFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGPlayerController>(ActorInfo->PlayerController.Get());
	}
	return nullptr;
}

void URPGPlayerAbility_DeathBase::StartDeathSequence_Implementation(AActor* AvatarActor)
{
	// 先执行共享死亡逻辑（禁用碰撞、停止移动、通知接口）
	Super::StartDeathSequence_Implementation(AvatarActor);

	ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(AvatarActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogRPGPlayerDeath, Log, TEXT("Player DeathStarted: %s"), *PlayerCharacter->GetName());

	// 禁用玩家输入
	if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		PC->DisableInput(PC);
	}

	// 调用蓝图可重写钩子
	OnPlayerDeathStarted(PlayerCharacter);
}

void URPGPlayerAbility_DeathBase::FinishDeathSequence_Implementation(AActor* AvatarActor)
{
	// 先执行共享死亡完成逻辑
	Super::FinishDeathSequence_Implementation(AvatarActor);

	ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(AvatarActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogRPGPlayerDeath, Log, TEXT("Player DeathFinished: %s"), *PlayerCharacter->GetName());

	// 调用蓝图可重写钩子
	OnPlayerDeathFinished(PlayerCharacter);
}

void URPGPlayerAbility_DeathBase::OnPlayerDeathStarted_Implementation(ARPGPlayerCharacter* PlayerCharacter)
{
	// 默认实现为空，蓝图子类可重写以添加 UI 表现（如屏幕变灰、死亡提示等）
}

void URPGPlayerAbility_DeathBase::OnPlayerDeathFinished_Implementation(ARPGPlayerCharacter* PlayerCharacter)
{
	// 默认实现为空，蓝图子类可重写以触发重生流程或显示死亡 UI
}
