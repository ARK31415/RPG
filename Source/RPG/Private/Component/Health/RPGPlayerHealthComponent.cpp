// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Health/RPGPlayerHealthComponent.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerHealthComponent, Log, All)

URPGPlayerHealthComponent::URPGPlayerHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsInvincible = false;
	InvincibleDuration = 2.0f; // 默认无敌时间 2 秒
}

void URPGPlayerHealthComponent::SetInvincible(bool bInInvincible)
{
	bIsInvincible = bInInvincible;
	
	if (bInInvincible)
	{
		UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player became invincible"));
	}
	else
	{
		UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player lost invincibility"));
	}
}

void URPGPlayerHealthComponent::Revive(float HealthPercent)
{
	if (!IsDead())
	{
		UE_LOG(LogRPGPlayerHealthComponent, Warning, TEXT("Cannot revive: Player is not dead"));
		return;
	}

	// 重置死亡状态
	bIsInvincible = false;
	
	// 恢复生命值
	float ReviveHealth = GetMaxHealth() * HealthPercent;
	CurrentHealth = ReviveHealth;
	
	// 通过 ASC 更新属性
	if (AbilitySystemComponent)
	{
		// TODO: 使用 GE 来恢复生命值
		// 这里直接修改属性作为临时方案
	}

	UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player revived with %.0f HP (%.0f%%)"), 
		ReviveHealth, HealthPercent * 100.0f);

	// 广播死亡完成事件（结束死亡状态）
	OnDeathFinished.Broadcast();
}

void URPGPlayerHealthComponent::StartDeath()
{
	// 如果无敌，不触发死亡
	if (bIsInvincible)
	{
		UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player is invincible, death prevented"));
		CurrentHealth = 1.0f; // 至少保留 1 点血
		return;
	}

	Super::StartDeath();
	
	UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player death started"));
}

void URPGPlayerHealthComponent::FinishDeath()
{
	Super::FinishDeath();
	
	UE_LOG(LogRPGPlayerHealthComponent, Log, TEXT("Player death finished"));
	
	// 玩家死亡后的逻辑（例如：触发重生 UI、游戏结束等）
}
