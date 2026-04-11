// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/RPGAbility_HeavyAttackCombo.h"
#include "RPGAbility_HeavyAttack_Sword.generated.h"

/**
 * 剑重击连招（Player层 - 武器具体实现）
 * 通过CDO配置剑的重击Montage序列、段数等
 * 在蓝图子类或CDO中设置ComboMontages和MaxComboCount
 */
UCLASS()
class RPG_API URPGAbility_HeavyAttack_Sword : public URPGAbility_HeavyAttackCombo
{
	GENERATED_BODY()

public:
	URPGAbility_HeavyAttack_Sword();
};
