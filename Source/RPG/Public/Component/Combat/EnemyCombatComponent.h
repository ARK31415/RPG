// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"

/**
 * 敌人战斗组件
 * 重写命中检测和碰撞控制，提供敌人特有的战斗行为
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	virtual void OnHitTargetActor(AActor* HitActor) override;

	virtual void OnWeaponPullerFromTargetActor(AActor* InteractedActor) override;

	// TODO: 敌人特有碰撞控制（Body碰撞盒等）
	// virtual void ToggleBodyCollisionBoxCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType);
};
