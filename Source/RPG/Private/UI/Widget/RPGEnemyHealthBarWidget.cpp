// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/RPGEnemyHealthBarWidget.h"
#include "Component/UI/RPGEnemyUIComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
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
			SubscribeToEvents();
			
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

	// 验证组件绑定(开发期调试用)
	if (!HealthBar)
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Error, TEXT("HealthBar is not bound in Blueprint"));
	}
	if (!HealthText)
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Warning, TEXT("HealthText is not bound in Blueprint"));
	}
	if (!EnemyNameText)
	{
		UE_LOG(LogRPGEnemyHealthBarWidget, Warning, TEXT("EnemyNameText is not bound in Blueprint"));
	}

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

	EnemyUIComponent->OnHealthChangedForUI.AddUniqueDynamic(
		this, &URPGEnemyHealthBarWidget::OnEnemyHealthChangedDynamic);

	UpdateWidgetValues();

	UE_LOG(LogRPGEnemyHealthBarWidget, Log, TEXT("Subscribed to EnemyUIComponent events"));
}

void URPGEnemyHealthBarWidget::OnEnemyHealthChangedDynamic(float NewHealth, float OldHealth)
{
	if (!EnemyUIComponent.IsValid())
	{
		return;
	}

	UpdateWidgetValues();
	UpdateVisibility();
}

void URPGEnemyHealthBarWidget::UpdateVisibility()
{
	if (!EnemyUIComponent.IsValid())
	{
		return;
	}

	bool bShouldShow = EnemyUIComponent->ShouldShowHealthBar();

	if (UWidget* RootWidget = GetRootWidget())
	{
		RootWidget->SetVisibility(bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void URPGEnemyHealthBarWidget::UpdateWidgetValues()
{
	if (!EnemyUIComponent.IsValid())
	{
		return;
	}

	const float CurrentHealth = EnemyUIComponent->GetCurrentHealth();
	const float MaxHealth = EnemyUIComponent->GetMaxHealth();
	const float Percent = EnemyUIComponent->GetHealthPercent();

	UE_LOG(LogRPGEnemyHealthBarWidget, Log,
		TEXT("UpdateWidgetValues: Health=%.0f/%.0f (%.0f%%), HealthBar=%s, HealthText=%s, EnemyNameText=%s, Name=%s"),
		CurrentHealth, MaxHealth, Percent * 100.f,
		HealthBar ? TEXT("Valid") : TEXT("NULL"),
		HealthText ? TEXT("Valid") : TEXT("NULL"),
		EnemyNameText ? TEXT("Valid") : TEXT("NULL"),
		*EnemyUIComponent->GetEnemyDisplayName());

	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}

	if (HealthText)
	{
		HealthText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f/%.0f"), CurrentHealth, MaxHealth)));
	}

	if (EnemyNameText)
	{
		EnemyNameText->SetText(FText::FromString(
			EnemyUIComponent->GetEnemyDisplayName()));
	}

	BP_OnEnemyUIComponentInitialized(EnemyUIComponent.Get());
	BP_UpdateHealthBar(CurrentHealth, MaxHealth);
}
