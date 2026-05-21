// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/UI/PawnUIComponent.h"
#include "RPGEnemyUIComponent.generated.h"

class UUserWidget;

/**
 * Enemy UI Component
 * 职责：敌人特有的 UI 数据桥接（名称、等级、距离检测、血条可见性等）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGEnemyUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	URPGEnemyUIComponent();

	/** 血条是否应该显示（基于距离和生命值） */
	UFUNCTION(BlueprintPure, Category="Enemy UI")
	bool ShouldShowHealthBar() const;

	/** 获取敌人名称（用于 UI 显示） */
	UFUNCTION(BlueprintPure, Category="Enemy UI")
	FString GetEnemyDisplayName() const;

	/** 获取敌人等级 */
	UFUNCTION(BlueprintPure, Category="Enemy UI")
	int32 GetEnemyLevel() const;

	/** 初始化血条 Widget（BeginPlay 和对象池激活时调用） */
	UFUNCTION(BlueprintCallable, Category="Enemy UI")
	void InitializeHealthBarWidget();

	/** 重置血条 Widget（对象池重新激活时调用） */
	UFUNCTION(BlueprintCallable, Category="Enemy UI")
	void ResetHealthBarWidget();

protected:
	virtual URPGHealthComponent* GetHealthComponentInternal() const override;

	/** Enemy 特有的 BeginPlay 逻辑 */
	virtual void BeginPlay() override;

	/** 重写健康变化回调，添加距离检测逻辑 */
	virtual void OnHealthChangedInternal(float NewHealth, float OldHealth) override;

private:
	/** 显示血条的最大距离 */
	UPROPERTY(EditDefaultsOnly, Category="Enemy UI|Visibility")
	float MaxHealthBarDistance;

	/** 满血时是否隐藏血条 */
	UPROPERTY(EditDefaultsOnly, Category="Enemy UI|Visibility")
	bool bHideHealthBarWhenFull;

	/** 敌人显示名称 */
	UPROPERTY(EditDefaultsOnly, Category="Enemy UI|Display")
	FString DisplayName;

	/** 敌人等级 */
	UPROPERTY(EditDefaultsOnly, Category="Enemy UI|Display")
	int32 EnemyLevel;

	/** 缓存 WidgetComponent 引用（从 Owner 获取） */
	TWeakObjectPtr<class UWidgetComponent> CachedHealthBarWidgetComponent;

	/** 血条 Widget 蓝图类（优先使用，未设置时回退到 Character::HealthBarWidgetClass） */
	UPROPERTY(EditDefaultsOnly, Category="Enemy UI|Widget")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;
};
