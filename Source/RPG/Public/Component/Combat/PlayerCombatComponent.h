// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.h"
#include "Types/RPGEnumTypes.h"
#include "Types/RPGStructTypes.h"
#include "PlayerCombatComponent.generated.h"

class ARPGPlayerWeapon;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	ARPGPlayerWeapon* GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const;

	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	ARPGPlayerWeapon* GetPlayerCurrentEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category="RPG|Combat")
	float GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLeveL) const;

	virtual void OnHitTargetActor(AActor* HitActor) override;
	virtual void OnWeaponPullerFromTargetActor(AActor* InteractedActor) override;

	// ========== 连招系统 ==========

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void StartAttack();

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void TryComboAttack();

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void ResetCombo();

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void CloseComboWindow();

	// ========== 连招状态查询 ==========

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	int32 GetComboIndex() const { return ComboIndex; }

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	int32 GetMaxComboCount() const { return MaxComboCount; }

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	ERPGCombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintPure, Category="RPG|Combo")
	float GetAttackSpeedMultiplier() const { return AttackSpeedMultiplier; }

	// ========== 设置接口 ==========

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SetMaxComboCount(int32 InMaxComboCount);

	UFUNCTION(BlueprintCallable, Category="RPG|Combo")
	void SetAttackSpeedMultiplier(float NewMultiplier);

	// 从武器数据应用战斗参数
	void ApplyWeaponData(const FRPGPlayerWeaponData& WeaponData);

protected:
	UPROPERTY(BlueprintReadOnly, Category="RPG|Combo")
	ERPGCombatState CombatState;

	UPROPERTY(BlueprintReadOnly, Category="RPG|Combo")
	int32 ComboIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RPG|Combo")
	int32 MaxComboCount;

	UPROPERTY(BlueprintReadOnly, Category="RPG|Combo")
	bool bIsInComboWindow;

	UPROPERTY(BlueprintReadOnly, Category="RPG|Combo")
	float AttackSpeedMultiplier;
};
