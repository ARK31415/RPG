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

	// 绑定Widget组件
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

	// 获取PlayerState并绑定属性变化
	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC && PC->GetPlayerState<ARPGPlayerState>())
	{
		ARPGPlayerState* PS = Cast<ARPGPlayerState>(PC->GetPlayerState<ARPGPlayerState>());
		if (PS && PS->GetRPGAttributeSet())
		{
			URPGAttributeSet* AttributeSet = PS->GetRPGAttributeSet();

			// 绑定生命值变化
			HealthChangedDelegateHandle = AttributeSet->OnHealthChanged.AddUObject(this, &URPGHUDWidget::UpdateHealth);

			// 绑定法力值变化
			ManaChangedDelegateHandle = AttributeSet->OnManaChanged.AddUObject(this, &URPGHUDWidget::UpdateMana);

			// 初始化显示
			UpdateHealth(AttributeSet->GetCurrentHealth(), AttributeSet->GetMaxHealth());
			UpdateMana(AttributeSet->GetCurrentMana(), AttributeSet->GetMaxMana());
		}
	}
}

void URPGHUDWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 清理Delegate绑定
	if (HealthChangedDelegateHandle.IsValid())
	{
		URPGAttributeSet* AttributeSet = nullptr;
		APlayerController* PC = GetOwningPlayer<APlayerController>();
		if (PC && PC->GetPlayerState<ARPGPlayerState>())
		{
			ARPGPlayerState* PS = Cast<ARPGPlayerState>(PC->GetPlayerState<ARPGPlayerState>());
			if (PS && PS->GetRPGAttributeSet())
			{
				AttributeSet = PS->GetRPGAttributeSet();
			}
		}

		if (AttributeSet)
		{
			AttributeSet->OnHealthChanged.Remove(HealthChangedDelegateHandle);
		}
		HealthChangedDelegateHandle.Reset();
	}

	if (ManaChangedDelegateHandle.IsValid())
	{
		URPGAttributeSet* AttributeSet = nullptr;
		APlayerController* PC = GetOwningPlayer<APlayerController>();
		if (PC && PC->GetPlayerState<ARPGPlayerState>())
		{
			ARPGPlayerState* PS = Cast<ARPGPlayerState>(PC->GetPlayerState<ARPGPlayerState>());
			if (PS && PS->GetRPGAttributeSet())
			{
				AttributeSet = PS->GetRPGAttributeSet();
			}
		}

		if (AttributeSet)
		{
			AttributeSet->OnManaChanged.Remove(ManaChangedDelegateHandle);
		}
		ManaChangedDelegateHandle.Reset();
	}
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
