// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RPGWidget_ActivatableBase.h"
#include "Interface/PawnUIInterface.h"
#include "Component/UI/RPGPlayerUIComponent.h"
#include "RPGHUDWidget.generated.h"
/**
 * HUD Widget，显示玩家状态信息
 */
UCLASS()
class RPG_API URPGHUDWidget : public URPGWidget_ActivatableBase
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

	/** 当 PlayerUIComponent 初始化完成时调用（蓝图实现） */
	UFUNCTION(BlueprintImplementableEvent, Category="UI")
	void BP_OnPlayerUIComponentInitialized(URPGPlayerUIComponent* NewPlayerUIComponent);

	/** 动态委托回调函数（必须是 UFUNCTION） */
	UFUNCTION()
	void OnHealthChangedDynamic(float NewHealth, float OldHealth);

	UFUNCTION()
	void OnManaChangedDynamic(float NewMana, float OldMana);

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
	/** Player UI 组件引用 */
	TWeakObjectPtr<URPGPlayerUIComponent> PlayerUIComponent;
};
