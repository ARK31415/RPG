// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/RPGEnemyHealthBarWidget.h"
#include "Component/UI/RPGEnemyUIComponent.h"
#include "Components/Widget.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyHealthBarWidget, Log, All)

void URPGEnemyHealthBarWidget::InitEnemyCreatedWidget(AActor* OwningEnemyActor)
{
	if (!OwningEnemyActor)
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Error, TEXT("InitEnemyCreatedWidget: OwningEnemyActor is null"));
		return;
	}

	// 通过 Interface 获取 EnemyUIComponent
	if (IPawnUIInterface* UIInterface = Cast<IPawnUIInterface>(OwningEnemyActor))
	{
		URPGEnemyUIComponent* UIComp = UIInterface->GetEnemyUIComponent();
		
		if (UIComp)
		{
			EnemyUIComponent = UIComp;
			BP_OnEnemyUIComponentInitialized(UIComp);
			
			UE_LOG(LogRPGEnemyHealthBarWidget, Log, TEXT("EnemyHealthBarWidget: EnemyUIComponent acquired for %s"), 
				*OwningEnemyActor->GetName());
		}
		else
		{
			UE_LOG(LogRPGEnemyHealthBarWidget, Error, TEXT("Failed to get EnemyUIComponent from %s"), 
				*OwningEnemyActor->GetName());
		}
	}
	else
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Error, TEXT("OwningEnemyActor does not implement IPawnUIInterface: %s"), 
			*OwningEnemyActor->GetName());
	}
}

void URPGEnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 如果已经绑定了 EnemyUIComponent，订阅事件
	if (EnemyUIComponent.IsValid())
	{
		SubscribeToEvents();
	}
}

void URPGEnemyHealthBarWidget::NativeDestruct()
{
	// 移除动态委托绑定
	if (EnemyUIComponent.IsValid())
	{
		EnemyUIComponent->OnHealthChangedForUI.RemoveDynamic(
			this, &URPGEnemyHealthBarWidget::OnEnemyHealthChangedDynamic);
	}
	
	Super::NativeDestruct();
}

void URPGEnemyHealthBarWidget::SubscribeToEvents()
{
	if (!EnemyUIComponent.IsValid())
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Warning, TEXT("SubscribeToEvents: EnemyUIComponent is null"));
		return;
	}

	// 订阅生命值变化（使用 AddUniqueDynamic）
	EnemyUIComponent->OnHealthChangedForUI.AddUniqueDynamic(
		this, &URPGEnemyHealthBarWidget::OnEnemyHealthChangedDynamic);

	// 初始化显示
	OnEnemyHealthChangedDynamic(EnemyUIComponent->GetCurrentHealth(), EnemyUIComponent->GetMaxHealth());

	UE_LOG(LogRPGEnemyHealthBarWidget, Log, TEXT("Subscribed to EnemyUIComponent events"));
}

void URPGEnemyHealthBarWidget::OnEnemyHealthChangedDynamic(float NewHealth, float OldHealth)
{
	if (!EnemyUIComponent.IsValid())
	{
		return;
	}

	float MaxHealth = EnemyUIComponent->GetMaxHealth();

	// 调用蓝图实现的血条更新
	BP_UpdateHealthBar(NewHealth, MaxHealth);

	// 更新可见性
	UpdateVisibility();
}

void URPGEnemyHealthBarWidget::UpdateVisibility()
{
	if (!EnemyUIComponent.IsValid())
	{
		return;
	}

	// 使用 EnemyUIComponent 的可见性判断逻辑
	bool bShouldShow = EnemyUIComponent->ShouldShowHealthBar();
	
	// 设置 Widget 可见性
	if (UWidget* RootWidget = GetRootWidget())
	{
		RootWidget->SetVisibility(bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
