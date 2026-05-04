// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/PawnExtensionComponentBase.h"
#include "PawnUIComponent.generated.h"

// ========== 自定义委托声明 ==========
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedForUIDelegate, float, NewHealth, float, OldHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChangedForUIDelegate, float, NewMaxHealth, float, OldMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathStartedForUIDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathFinishedForUIDelegate);



class URPGHealthComponent;

/**
 * Pawn UI Component 基类
 * 职责：订阅 HealthComponent 事件，转换为 UI 友好格式，提供通用 UI 数据访问
 * 子类继承后可添加特定于 Player/Enemy 的 UI 逻辑
 */
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UPawnUIComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	UPawnUIComponent();

	/** 获取当前生命值 */
	UFUNCTION(BlueprintPure, Category="UI")
	float GetCurrentHealth() const;

	/** 获取最大生命值 */
	UFUNCTION(BlueprintPure, Category="UI")
	float GetMaxHealth() const;

	/** 获取生命值百分比 (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category="UI")
	float GetHealthPercent() const;

	/** 是否已死亡 */
	UFUNCTION(BlueprintPure, Category="UI")
	bool IsDead() const;

	// ========== UI 事件广播 ==========

	/** 生命值变化事件 */
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnHealthChangedForUIDelegate OnHealthChangedForUI;

	/** 最大生命值变化事件 */
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnMaxHealthChangedForUIDelegate OnMaxHealthChangedForUI;

	/** 死亡开始事件（转发给 UI） */
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnDeathStartedForUIDelegate OnDeathStartedForUI;

	/** 死亡完成事件（转发给 UI） */
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnDeathFinishedForUIDelegate OnDeathFinishedForUI;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 获取 HealthComponent（虚函数，子类必须实现） */
	virtual URPGHealthComponent* GetHealthComponentInternal() const { return nullptr; }

	/** 订阅 HealthComponent 事件 */
	void SubscribeToHealthComponent();

	/** 解除订阅 */
	void UnsubscribeFromHealthComponent();

	/** HealthComponent 事件回调（子类可重写以添加自定义逻辑） */
	virtual void OnHealthChangedInternal(float NewHealth, float OldHealth);
	virtual void OnMaxHealthChangedInternal(float NewMaxHealth, float OldMaxHealth);
	virtual void OnDeathStartedInternal();
	virtual void OnDeathFinishedInternal();

	/** 动态委托回调函数（必须是 UFUNCTION） */
	UFUNCTION()
	void OnHealthChangedDynamic(float NewHealth, float OldHealth);

	UFUNCTION()
	void OnMaxHealthChangedDynamic(float NewMaxHealth, float OldMaxHealth);

	UFUNCTION()
	void OnDeathStartedDynamic();

	UFUNCTION()
	void OnDeathFinishedDynamic();

	UPROPERTY()
	TObjectPtr<URPGHealthComponent> HealthComponent;
};
