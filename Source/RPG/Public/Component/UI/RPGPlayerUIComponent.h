// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/MulticastDelegateBase.h"
#include "Component/UI/PawnUIComponent.h"
#include "RPGPlayerUIComponent.generated.h"

// ========== 自定义委托声明 ==========
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float, NewMana, float, OldMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentWeaponChangedDelegate);



/**
 * Player UI Component
 * 职责：玩家特有的 UI 数据桥接（武器、技能、Mana 等）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGPlayerUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	URPGPlayerUIComponent();

	// ========== Player 特有 UI 事件 ==========
	
	/** 法力值变化事件 */
	UPROPERTY(BlueprintAssignable, Category="Player UI|Events")
	FOnManaChangedDelegate OnManaChangedForUI;

	/** 当前武器变化事件 */
	UPROPERTY(BlueprintAssignable, Category="Player UI|Events")
	FOnCurrentWeaponChangedDelegate OnCurrentWeaponChangedForUI;

protected:
	virtual URPGHealthComponent* GetHealthComponentInternal() const override;

	/** Player 特有的 BeginPlay 逻辑 */
	virtual void BeginPlay() override;

private:
	/** 初始化 Mana 系统 */
	void InitializeManaSystem();

	// ========== Player 特有数据 ==========

	/** 当前法力值 */
	UPROPERTY(EditDefaultsOnly, Category="Player UI|Mana")
	float CurrentMana;

	/** 最大法力值 */
	UPROPERTY(EditDefaultsOnly, Category="Player UI|Mana")
	float MaxMana;
};
