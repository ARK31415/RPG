// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGEnumTypes.generated.h"

/**
 * 武器类型枚举
 */
UENUM(BlueprintType)
enum class ERPGWeaponType : uint8
{
	None			UMETA(DisplayName = "无"),
	Sword1H			UMETA(DisplayName = "单手剑"),
	Sword2H			UMETA(DisplayName = "双手剑"),
	Bow				UMETA(DisplayName = "弓"),
	Staff			UMETA(DisplayName = "法杖"),
	DualBlade		UMETA(DisplayName = "双刀"),
	Spear			UMETA(DisplayName = "长枪")
};

/**
 * 角色职业枚举
 */
UENUM(BlueprintType)
enum class ERPGCharacterClass : uint8
{
	None			UMETA(DisplayName = "无"),
	RPG			UMETA(DisplayName = "战士"),
	Mage			UMETA(DisplayName = "法师"),
	Archer			UMETA(DisplayName = "弓箭手"),
	Assassin		UMETA(DisplayName = "刺客"),
	Paladin			UMETA(DisplayName = "圣骑士")
};

/**
 * 战斗状态枚举（通用）
 */
UENUM(BlueprintType)
enum class ERPGCombatState : uint8
{
	Idle			UMETA(DisplayName = "待机"),
	Combat			UMETA(DisplayName = "战斗中"),
	Attacking		UMETA(DisplayName = "攻击中"),
	Blocking		UMETA(DisplayName = "格挡中"),
	Dodging			UMETA(DisplayName = "闪避中"),
	Stunned			UMETA(DisplayName = "眩晕中"),
	Dead			UMETA(DisplayName = "死亡")
};

/**
 * 连招攻击类型枚举
 * 用于区分不同攻击类型的连招计数通道
 */
UENUM(BlueprintType)
enum class ERPGComboType : uint8
{
	LightAttack		UMETA(DisplayName = "轻击"),
	HeavyAttack		UMETA(DisplayName = "重击")
};

UENUM()
enum class ERPGConfirmType : uint8
{
	Yes,
	No
};

UENUM()
enum class ERPGValidType : uint8
{
	Valid,
	InValid
};

UENUM()
enum class ERPGSuccessType : uint8
{
	Successful,
	Failed
};

/**
 * 敌人受击方向枚举
 */
UENUM(BlueprintType)
enum class EEnemyHitReactDirection : uint8
{
	Front	UMETA(DisplayName = "正面"),
	Back	UMETA(DisplayName = "背面"),
	Left	UMETA(DisplayName = "左侧"),
	Right	UMETA(DisplayName = "右侧")
};