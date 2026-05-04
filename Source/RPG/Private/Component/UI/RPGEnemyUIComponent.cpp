// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/UI/RPGEnemyUIComponent.h"
#include "Component/Health/RPGEnemyHealthComponent.h"
#include "Character/RPGEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyUIComponent, Log, All)

URPGEnemyUIComponent::URPGEnemyUIComponent()
{
	MaxHealthBarDistance = 2000.0f; // 20 米
	bHideHealthBarWhenFull = true;
	DisplayName = TEXT("Enemy");
	EnemyLevel = 1;
}

URPGHealthComponent* URPGEnemyUIComponent::GetHealthComponentInternal() const
{
	// 从 Enemy Character 获取 EnemyHealthComponent
	if (const ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(GetOwner()))
	{
		return EnemyCharacter->GetHealthComponent();
	}
	return nullptr;
}

void URPGEnemyUIComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogRPGEnemyUIComponent, Log, TEXT("EnemyUIComponent initialized for %s (Level %d)"), 
		*GetOwner()->GetName(), EnemyLevel);
}

void URPGEnemyUIComponent::OnHealthChangedInternal(float NewHealth, float OldHealth)
{
	// 调用基类实现（广播给 UI）
	Super::OnHealthChangedInternal(NewHealth, OldHealth);

	// Enemy 特有逻辑：可以根据血量改变血条颜色等
}

bool URPGEnemyUIComponent::ShouldShowHealthBar() const
{
	if (!HealthComponent)
	{
		return false;
	}

	// 死亡时隐藏
	if (HealthComponent->IsDead())
	{
		return false;
	}

	// 满血时隐藏
	if (bHideHealthBarWhenFull && HealthComponent->GetCurrentHealth() >= HealthComponent->GetMaxHealth())
	{
		return false;
	}

	// 距离检测
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
		if (Distance > MaxHealthBarDistance)
		{
			return false;
		}
	}

	return true;
}

FString URPGEnemyUIComponent::GetEnemyDisplayName() const
{
	return DisplayName;
}

int32 URPGEnemyUIComponent::GetEnemyLevel() const
{
	return EnemyLevel;
}
