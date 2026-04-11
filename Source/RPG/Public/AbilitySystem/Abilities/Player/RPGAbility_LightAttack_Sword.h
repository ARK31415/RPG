// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/RPGAbility_LightAttackCombo.h"
#include "RPGAbility_LightAttack_Sword.generated.h"

/**
 * 剑轻击连招（Player层 - 武器具体实现）
 * 通过CDO配置剑的轻击Montage序列、段数等
 * 在蓝图子类或CDO中设置ComboMontages和MaxComboCount
 */
UCLASS()
class RPG_API URPGAbility_LightAttack_Sword : public URPGAbility_LightAttackCombo
{
	GENERATED_BODY()

public:
	URPGAbility_LightAttack_Sword();
};
