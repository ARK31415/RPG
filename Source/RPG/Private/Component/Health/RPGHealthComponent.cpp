// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Health/RPGHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Character/BaseCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGHealthComponent, Log, All)

URPGHealthComponent::URPGHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	CurrentHealth = 0.0f;
	MaxHealth = 0.0f;
	bIsDead = false;
}

void URPGHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// 在 BeginPlay 中自动初始化
	ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner());
	if (Owner && Owner->GetAbilitySystemComponent())
	{
		InitializeWithAbilitySystem(Owner->GetAbilitySystemComponent());
	}
	else
	{
		UE_LOG(LogRPGHealthComponent, Warning, TEXT("HealthComponent: Owner or ASC is null for %s"), *GetOwner()->GetName());
	}
}

void URPGHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理 Delegate
	if (HealthChangedDelegateHandle.IsValid() && AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			URPGAttributeSet::GetCurrentHealthAttribute()
		).Remove(HealthChangedDelegateHandle);
	}

	if (MaxHealthChangedDelegateHandle.IsValid() && AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			URPGAttributeSet::GetMaxHealthAttribute()
		).Remove(MaxHealthChangedDelegateHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void URPGHealthComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogRPGHealthComponent, Error, TEXT("InitializeWithAbilitySystem: ASC is null"));
		return;
	}

	AbilitySystemComponent = ASC;

	// 绑定生命值属性变化（使用 GAS 原生 Delegate）
	HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		URPGAttributeSet::GetCurrentHealthAttribute()
	).AddUObject(this, &URPGHealthComponent::OnHealthAttributeChanged);

	MaxHealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		URPGAttributeSet::GetMaxHealthAttribute()
	).AddUObject(this, &URPGHealthComponent::OnMaxHealthAttributeChanged);

	// 初始化当前值
	const URPGAttributeSet* AttributeSet = ASC->GetSet<URPGAttributeSet>();
	if (AttributeSet)
	{
		CurrentHealth = AttributeSet->GetCurrentHealth();
		MaxHealth = AttributeSet->GetMaxHealth();

		UE_LOG(LogRPGHealthComponent, Log, TEXT("HealthComponent initialized: Health=%.0f/%.0f"), 
			CurrentHealth, MaxHealth);
	}
	else
	{
		UE_LOG(LogRPGHealthComponent, Error, TEXT("Failed to get AttributeSet from ASC"));
	}
}

void URPGHealthComponent::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;

	// 广播给 UIComponent
	OnHealthChanged.Broadcast(Data.NewValue, Data.OldValue);

	// 检测死亡
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		StartDeath();
	}
}

void URPGHealthComponent::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;

	// 广播给 UIComponent
	OnMaxHealthChanged.Broadcast(Data.NewValue, Data.OldValue);
}

void URPGHealthComponent::StartDeath()
{
	if (bIsDead)
	{
		UE_LOG(LogRPGHealthComponent, Warning, TEXT("StartDeath: Already dead for %s"), *GetOwner()->GetName());
		return;
	}
	
	bIsDead = true;
	
	UE_LOG(LogRPGHealthComponent, Log, TEXT("StartDeath: %s"), *GetOwner()->GetName());

	OnDeathStarted.Broadcast();

	// 延迟后完成死亡（给动画时间）
	GetWorld()->GetTimerManager().SetTimer(
		DeathFinishTimerHandle,
		this,
		&URPGHealthComponent::FinishDeath,
		2.0f  // 2秒后清理
	);
}

void URPGHealthComponent::FinishDeath()
{
	UE_LOG(LogRPGHealthComponent, Log, TEXT("FinishDeath: %s"), *GetOwner()->GetName());

	OnDeathFinished.Broadcast();
}
