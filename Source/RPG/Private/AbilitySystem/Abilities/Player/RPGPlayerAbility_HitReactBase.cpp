// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/RPGPlayerAbility_HitReactBase.h"
#include "Character/RPGPlayerCharacter.h"
#include "Controllers/RPGPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerHitReact, Log, All)

ARPGPlayerCharacter* URPGPlayerAbility_HitReactBase::GetPlayerCharacterFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGPlayerCharacter>(ActorInfo->AvatarActor.Get());
	}
	return nullptr;
}

ARPGPlayerController* URPGPlayerAbility_HitReactBase::GetPlayerControllerFromActorInfo() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		return Cast<ARPGPlayerController>(ActorInfo->PlayerController.Get());
	}
	return nullptr;
}

void URPGPlayerAbility_HitReactBase::StartHitReactSequence_Implementation(AActor* AvatarActor, FGameplayTag HitDirection)
{
	// 先执行共享受击逻辑（停止移动）
	Super::StartHitReactSequence_Implementation(AvatarActor, HitDirection);

	ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(AvatarActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogRPGPlayerHitReact, Log, TEXT("Player HitReactStarted: %s, Direction: %s"),
		*PlayerCharacter->GetName(), *HitDirection.ToString());

	// 调用蓝图可重写钩子
	OnPlayerHitReactStarted(PlayerCharacter, HitDirection);
}

void URPGPlayerAbility_HitReactBase::FinishHitReactSequence_Implementation(AActor* AvatarActor)
{
	// 先执行共享受击完成逻辑
	Super::FinishHitReactSequence_Implementation(AvatarActor);

	ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(AvatarActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogRPGPlayerHitReact, Log, TEXT("Player HitReactFinished: %s"), *PlayerCharacter->GetName());

	// 调用蓝图可重写钩子
	OnPlayerHitReactFinished(PlayerCharacter);
}

void URPGPlayerAbility_HitReactBase::OnPlayerHitReactStarted_Implementation(ARPGPlayerCharacter* PlayerCharacter, FGameplayTag HitDirection)
{
	// 默认实现为空，蓝图子类可重写以添加摄像机震动、UI 提示等
}

void URPGPlayerAbility_HitReactBase::OnPlayerHitReactFinished_Implementation(ARPGPlayerCharacter* PlayerCharacter)
{
	// 默认实现为空，蓝图子类可重写
}
