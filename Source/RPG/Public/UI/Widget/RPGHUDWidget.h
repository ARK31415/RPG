// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RPGHUDWidget.generated.h"
/**
 * HUD Widget，显示玩家状态信息
 */
UCLASS()
class RPG_API URPGHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// 构造函数
	URPGHUDWidget(const FObjectInitializer& ObjectInitializer);

	// 初始化Widget
	virtual bool Initialize() override;

	// 激活Widget
	virtual void NativeOnActivated() override;

	// 停用Widget
	virtual void NativeOnDeactivated() override;

	// 更新生命值显示
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	// 更新法力值显示
	void UpdateMana(float CurrentMana, float MaxMana);

	// 更新武器图标
	void UpdateWeaponIcon(UTexture2D* WeaponIcon);

protected:
	// 生命值进度条
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* HealthBar;

	// 法力值进度条
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* ManaBar;

	// 生命值文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* HealthText;

	// 法力值文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* ManaText;

	// 武器图标
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* WeaponIcon;

private:
	// 绑定到PlayerState的Delegate
	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
};
