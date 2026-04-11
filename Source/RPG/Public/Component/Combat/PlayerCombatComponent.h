// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.h"
#include "Types/RPGStructTypes.h"
#include "PlayerCombatComponent.generated.h"

class ARPGPlayerWeapon;

/**
 * 玩家战斗组件
 * 职责：武器管理、命中检测、伤害计算
 * 连招逻辑已迁移至GAS Ability层（URPGAbility_AttackCombo继承链）
 */
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
};
