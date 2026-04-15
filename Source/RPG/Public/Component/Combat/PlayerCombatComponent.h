// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.h"
#include "Types/RPGStructTypes.h"
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

	// 连招状态管理
	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	int32 GetCurrentComboCount() const { return CurrentComboCount; }

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SetCurrentComboCount(int32 NewCount) { CurrentComboCount = NewCount; }

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void ResetComboCount();

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void AdvanceComboCount(int32 MaxComboCount);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 连招状态
	int32 CurrentComboCount = 1;

	// 连招窗口定时器
	FTimerHandle ComboResetTimerHandle;

	// 定时器回调
	UFUNCTION()
	void OnComboWindowTimerExpired();
};
