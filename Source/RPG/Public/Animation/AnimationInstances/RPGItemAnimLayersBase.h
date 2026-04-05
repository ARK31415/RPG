// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimationInstances/RPGBaseAnimInstance.h"
#include "Types/RPGEnumTypes.h"
#include "RPGItemAnimLayersBase.generated.h"

class UAnimMontage;
class UPlayerCombatComponent;

/**
 * 武器/物品动画层基类 - 纯数据接收与表现层
 * 每帧从 PlayerCombatComponent 读取数据，驱动动画蓝图
 */
UCLASS(Abstract)
class RPG_API URPGItemAnimLayersBase : public URPGBaseAnimInstance
{
	GENERATED_BODY()

public:
	URPGItemAnimLayersBase();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// CombatComponent 引用（NativeInitializeAnimation 中缓存）
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UPlayerCombatComponent> CombatComponent;

	// 武器类型（由子类或配置设置）
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	ERPGWeaponType WeaponType;

	// 以下属性每帧从 CombatComponent 同步
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ERPGCombatState CombatState;

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	int32 ComboIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	int32 MaxComboCount;

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	bool bIsInComboWindow;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AttackSpeedMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking;

public:
	// ========== 状态查询 ==========

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ERPGWeaponType GetWeaponType() const { return WeaponType; }

	// ========== 设置接口 ==========

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponType(ERPGWeaponType NewType);

private:
	void SyncFromCombatComponent();
};
