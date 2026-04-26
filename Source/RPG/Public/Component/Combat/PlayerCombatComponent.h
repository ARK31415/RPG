// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.h"
#include "Types/RPGStructTypes.h"
#include "Types/RPGEnumTypes.h"
#include "PlayerCombatComponent.generated.h"

class ARPGPlayerWeapon;

/**
 * 玩家战斗组件
 * 职责：武器管理、命中检测、伤害计算、连招状态管理
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	ARPGPlayerWeapon* GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const;

	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	ARPGPlayerWeapon* GetPlayerCurrentEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	float GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLeveL) const;

	virtual void OnHitTargetActor(AActor* HitActor) override;
	virtual void OnWeaponPullerFromTargetActor(AActor* InteractedActor) override;

	// 连招状态管理（按攻击类型分通道）
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	int32 GetComboCount(ERPGComboType ComboType) const;

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SetComboCount(ERPGComboType ComboType, int32 NewCount);

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void ResetComboCount(ERPGComboType ComboType);

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void AdvanceComboCount(ERPGComboType ComboType, int32 MaxComboCount);

	/** 切换攻击类型时重置对方计数器 */
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SwitchComboType(ERPGComboType NewComboType);

	/** 启动指定类型的连招窗口定时器 */
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void StartComboWindowTimer(ERPGComboType ComboType, float WindowTime);

	/** 获取当前攻击类型 (蓝图GA用于伤害计算) */
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	ERPGComboType GetCurrentComboType() const { return CurrentComboType; }

	/** 设置当前攻击类型 (攻击GA开始时调用) */
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SetCurrentComboType(ERPGComboType Type);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 连招状态（按攻击类型分通道）
	TMap<ERPGComboType, int32> ComboCounts;

	// 连招窗口定时器（按攻击类型分通道）
	TMap<ERPGComboType, FTimerHandle> ComboResetTimers;

	// 当前攻击类型 (用于伤害计算)
	ERPGComboType CurrentComboType = ERPGComboType::LightAttack;

	// 初始化默认连招计数
	void InitComboCounts();

	// 定时器回调
	void OnComboWindowTimerExpired(ERPGComboType ComboType);
};
