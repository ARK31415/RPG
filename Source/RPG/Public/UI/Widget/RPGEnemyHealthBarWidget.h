// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/PawnUIInterface.h"
#include "Component/UI/RPGEnemyUIComponent.h"
#include "RPGEnemyHealthBarWidget.generated.h"

/**
 * 敌人头顶血条 Widget
 * 职责：显示在 Enemy 头顶，通过 Interface 获取 EnemyUIComponent
 */
UCLASS()
class RPG_API URPGEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Enemy 生成时调用，绑定到敌人 */
	UFUNCTION(BlueprintCallable, Category="UI")
	void InitEnemyCreatedWidget(AActor* OwningEnemyActor);

	/** 获取 EnemyUIComponent */
	UFUNCTION(BlueprintPure, Category="UI")
	URPGEnemyUIComponent* GetEnemyUIComponent() const { return EnemyUIComponent.Get(); }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 当 EnemyUIComponent 初始化完成时调用（蓝图实现） */
	UFUNCTION(BlueprintImplementableEvent, Category="UI")
	void BP_OnEnemyUIComponentInitialized(URPGEnemyUIComponent* NewEnemyUIComponent);

	/** 更新血条显示 */
	UFUNCTION(BlueprintImplementableEvent, Category="UI")
	void BP_UpdateHealthBar(float CurrentHealth, float MaxHealth);

	/** 动态委托回调函数（必须是 UFUNCTION） */
	UFUNCTION()
	void OnEnemyHealthChangedDynamic(float NewHealth, float OldHealth);

private:
	/** Enemy UI 组件引用 */
	TWeakObjectPtr<URPGEnemyUIComponent> EnemyUIComponent;

	/** 订阅 EnemyUIComponent 事件 */
	void SubscribeToEvents();

	/** 更新血条可见性 */
	void UpdateVisibility();
};
