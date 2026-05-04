// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/UI/PawnUIComponent.h"
#include "Component/Health/RPGHealthComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogPawnUIComponent, Log, All)

UPawnUIComponent::UPawnUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UPawnUIComponent::GetCurrentHealth() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetCurrentHealth();
	}
	return 0.0f;
}

float UPawnUIComponent::GetMaxHealth() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetMaxHealth();
	}
	return 0.0f;
}

float UPawnUIComponent::GetHealthPercent() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetHealthPercent();
	}
	return 0.0f;
}

bool UPawnUIComponent::IsDead() const
{
	if (HealthComponent)
	{
		return HealthComponent->IsDead();
	}
	return false;
}

void UPawnUIComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取 HealthComponent
	HealthComponent = GetHealthComponentInternal();
	
	if (HealthComponent)
	{
		SubscribeToHealthComponent();
		UE_LOG(LogPawnUIComponent, Log, TEXT("PawnUIComponent: Subscribed to HealthComponent for %s"), 
			*GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogPawnUIComponent, Warning, TEXT("PawnUIComponent: No HealthComponent found for %s"), 
			*GetOwner()->GetName());
	}
}

void UPawnUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromHealthComponent();
	Super::EndPlay(EndPlayReason);
}

void UPawnUIComponent::SubscribeToHealthComponent()
{
	if (!HealthComponent)
	{
		return;
	}

	// 订阅 HealthComponent 的事件（使用 AddUniqueDynamic）
	HealthComponent->OnHealthChanged.AddUniqueDynamic(
		this, &UPawnUIComponent::OnHealthChangedDynamic);

	HealthComponent->OnMaxHealthChanged.AddUniqueDynamic(
		this, &UPawnUIComponent::OnMaxHealthChangedDynamic);

	HealthComponent->OnDeathStarted.AddUniqueDynamic(
		this, &UPawnUIComponent::OnDeathStartedDynamic);

	HealthComponent->OnDeathFinished.AddUniqueDynamic(
		this, &UPawnUIComponent::OnDeathFinishedDynamic);
}

void UPawnUIComponent::UnsubscribeFromHealthComponent()
{
	if (!HealthComponent)
	{
		return;
	}

	// 移除动态委托绑定
	HealthComponent->OnHealthChanged.RemoveDynamic(
		this, &UPawnUIComponent::OnHealthChangedDynamic);

	HealthComponent->OnMaxHealthChanged.RemoveDynamic(
		this, &UPawnUIComponent::OnMaxHealthChangedDynamic);

	HealthComponent->OnDeathStarted.RemoveDynamic(
		this, &UPawnUIComponent::OnDeathStartedDynamic);

	HealthComponent->OnDeathFinished.RemoveDynamic(
		this, &UPawnUIComponent::OnDeathFinishedDynamic);
}

void UPawnUIComponent::OnHealthChangedDynamic(float NewHealth, float OldHealth)
{
	OnHealthChangedInternal(NewHealth, OldHealth);
}

void UPawnUIComponent::OnMaxHealthChangedDynamic(float NewMaxHealth, float OldMaxHealth)
{
	OnMaxHealthChangedInternal(NewMaxHealth, OldMaxHealth);
}

void UPawnUIComponent::OnDeathStartedDynamic()
{
	OnDeathStartedInternal();
}

void UPawnUIComponent::OnDeathFinishedDynamic()
{
	OnDeathFinishedInternal();
}

void UPawnUIComponent::OnHealthChangedInternal(float NewHealth, float OldHealth)
{
	// 转发给 UI 层
	OnHealthChangedForUI.Broadcast(NewHealth, OldHealth);
}

void UPawnUIComponent::OnMaxHealthChangedInternal(float NewMaxHealth, float OldMaxHealth)
{
	// 转发给 UI 层
	OnMaxHealthChangedForUI.Broadcast(NewMaxHealth, OldMaxHealth);
}

void UPawnUIComponent::OnDeathStartedInternal()
{
	// 转发给 UI 层
	OnDeathStartedForUI.Broadcast();
}

void UPawnUIComponent::OnDeathFinishedInternal()
{
	// 转发给 UI 层
	OnDeathFinishedForUI.Broadcast();
}
