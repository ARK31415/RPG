// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/PawnExtensionComponentBase.h"
#include "Interface/PawnDeathInterface.h"
#include "RPGHealthComponent.generated.h"

// ========== 自定义委托声明 ==========
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, NewHealth, float, OldHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChangedDelegate, float, NewMaxHealth, float, OldMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathFinishedDelegate);



class UAbilitySystemComponent;

// 属性变化委托
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributeValueChanged, float, float);

/**
 * RPG Health Component 基类
 * 职责：管理生命值数据、监听 ASC 属性变化、广播健康状态事件
 * 这是一个数据组件，不直接处理 UI 逻辑
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGHealthComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	URPGHealthComponent();

	/** 初始化：绑定 ASC */
	UFUNCTION(BlueprintCallable, Category="Health")
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);



	// ========== 数据访问接口 ==========
	
	/** 获取当前生命值 */
	UFUNCTION(BlueprintPure, Category="Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	/** 获取最大生命值 */
	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** 是否已死亡 */
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const { return bIsDead; }

	/** 获取生命值百分比 (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }



	// ========== 事件广播（供 UIComponent 订阅）==========
	
	/** 生命值变化事件 */
	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnHealthChangedDelegate OnHealthChanged;
	
	/** 最大生命值变化事件 */
	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnMaxHealthChangedDelegate OnMaxHealthChanged;

	/** 死亡开始事件 */
	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnDeathStartedDelegate OnDeathStarted;
	
	/** 死亡完成事件 */
	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnDeathFinishedDelegate OnDeathFinished;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** ASC 属性变化回调 */
	virtual void OnHealthAttributeChanged(const struct FOnAttributeChangeData& Data);
	virtual void OnMaxHealthAttributeChanged(const struct FOnAttributeChangeData& Data);

	/** 死亡状态机（子类可重写） */
	virtual void StartDeath();
	virtual void FinishDeath();

	// ========== 子类可访问的成员 ==========

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;

	float CurrentHealth;
	float MaxHealth;
	bool bIsDead;

	FTimerHandle DeathFinishTimerHandle;

private:
};
