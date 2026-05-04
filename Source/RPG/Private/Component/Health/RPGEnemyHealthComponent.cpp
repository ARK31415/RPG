// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Health/RPGEnemyHealthComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyHealthComponent, Log, All)

URPGEnemyHealthComponent::URPGEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bPlayDeathAnimation = true;
	DeathAnimationDuration = 2.0f;
	bDestroyOnDeath = true;
	DestroyDelay = 3.0f;
}

void URPGEnemyHealthComponent::StartDeath()
{
	Super::StartDeath();
	
	UE_LOG(LogRPGEnemyHealthComponent, Log, TEXT("Enemy death started for %s"), *GetOwner()->GetName());

	// TODO: 播放死亡动画
	if (bPlayDeathAnimation)
	{
		// 播放死亡动画的逻辑
		// AnimationInstance->PlayDeathAnimation();
	}
}

void URPGEnemyHealthComponent::FinishDeath()
{
	Super::FinishDeath();
	
	UE_LOG(LogRPGEnemyHealthComponent, Log, TEXT("Enemy death finished for %s"), *GetOwner()->GetName());

	// 敌人死亡后的逻辑
	// 1. 掉落物品
	// 2. 给予经验值
	// 3. 播放音效
	
	// 如果设置了死亡后销毁
	if (bDestroyOnDeath)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (GetOwner())
			{
				GetOwner()->Destroy();
			}
		});
	}
}
