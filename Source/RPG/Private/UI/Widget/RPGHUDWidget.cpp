// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Widget/RPGHUDWidget.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.h"
#include "Character/RPGPlayerState.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGHUDWidget, All, All)

URPGHUDWidget::URPGHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 设置默认值
	HealthBar = nullptr;
	ManaBar = nullptr;
	HealthText = nullptr;
	ManaText = nullptr;
	WeaponIcon = nullptr;
}

bool URPGHUDWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// 通过 Interface 获取 PlayerUIComponent
	if (IPawnUIInterface* UIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
		URPGPlayerUIComponent* UIComp = UIInterface->GetPlayerUIComponent();
		if (UIComp)
		{
			PlayerUIComponent = UIComp;
			BP_OnPlayerUIComponentInitialized(UIComp);
			UE_LOG(LogRPGHUDWidget, Log, TEXT("HUD Widget: PlayerUIComponent acquired"));
		}
	}

	// 绑定 Widget 组件
	if (HealthBar)
	{
		HealthBar->SetPercent(1.0f);
	}

	if (ManaBar)
	{
		ManaBar->SetPercent(1.0f);
	}

	if (HealthText)
	{
		HealthText->SetText(FText::FromString(TEXT("HP: 100/100")));
	}

	if (ManaText)
	{
		ManaText->SetText(FText::FromString(TEXT("MP: 100/100")));
	}

	return true;
}

void URPGHUDWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!PlayerUIComponent.IsValid())
	{
		UE_LOG(LogRPGHUDWidget, Warning, TEXT("HUD Widget: PlayerUIComponent is null"));
		return;
	}

	// 订阅 PlayerUIComponent 的事件（使用 AddUniqueDynamic）
	PlayerUIComponent->OnHealthChangedForUI.AddUniqueDynamic(
		this, &URPGHUDWidget::OnHealthChangedDynamic);

	PlayerUIComponent->OnManaChangedForUI.AddUniqueDynamic(
		this, &URPGHUDWidget::OnManaChangedDynamic);

	// 初始化显示
	UpdateHealth(PlayerUIComponent->GetCurrentHealth(), PlayerUIComponent->GetMaxHealth());
	UpdateMana(100.0f, 100.0f); // TODO: 从 UIComponent 获取 Mana
}

void URPGHUDWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 移除动态委托绑定
	if (PlayerUIComponent.IsValid())
	{
		PlayerUIComponent->OnHealthChangedForUI.RemoveDynamic(
			this, &URPGHUDWidget::OnHealthChangedDynamic);

		PlayerUIComponent->OnManaChangedForUI.RemoveDynamic(
			this, &URPGHUDWidget::OnManaChangedDynamic);
	}
}

void URPGHUDWidget::OnHealthChangedDynamic(float NewHealth, float OldHealth)
{
	if (PlayerUIComponent.IsValid())
	{
		UpdateHealth(NewHealth, PlayerUIComponent->GetMaxHealth());
	}
}

void URPGHUDWidget::OnManaChangedDynamic(float NewMana, float OldMana)
{
	UpdateMana(NewMana, 100.0f); // TODO: 从 UIComponent 获取 MaxMana
}

void URPGHUDWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthBar)
	{
		float Percent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f;
		HealthBar->SetPercent(Percent);
	}

	if (HealthText)
	{
		FString HealthTextStr = FString::Printf(TEXT("HP: %.0f/%.0f"), CurrentHealth, MaxHealth);
		HealthText->SetText(FText::FromString(HealthTextStr));
	}
}

void URPGHUDWidget::UpdateMana(float CurrentMana, float MaxMana)
{
	if (ManaBar)
	{
		float Percent = MaxMana > 0 ? CurrentMana / MaxMana : 0.0f;
		ManaBar->SetPercent(Percent);
	}

	if (ManaText)
	{
		FString ManaTextStr = FString::Printf(TEXT("MP: %.0f/%.0f"), CurrentMana, MaxMana);
		ManaText->SetText(FText::FromString(ManaTextStr));
	}
}

void URPGHUDWidget::UpdateWeaponIcon(UTexture2D* WeaponIconTexture)
{
	if (WeaponIcon && WeaponIconTexture)
	{
		WeaponIcon->SetBrushFromTexture(WeaponIconTexture);
	}
}
