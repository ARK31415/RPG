#pragma once

#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "Animation/AnimMontage.h"

#include "RPGStructTypes.generated.h"

class URPGItemAnimLayersBase;
class URPGPlayerGameplayAbility;
class URPGGameplayAbility;
class UInputMappingContext;

/**
 * 角色基础属性配置
 * 用于 DataAsset 配置不同角色的初始属性值
 * 运行时通过 GameplayEffect 应用到 ASC
 */
USTRUCT(BlueprintType)
struct FCharacterBaseAttributes
{
	GENERATED_BODY()

	/*
	 * 主属性 (Primary)
	 */

	// 力量 - 影响物理攻击力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary Attributes")
	float Strength = 10.0f;

	// 智力 - 影响魔法攻击力/法力值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary Attributes")
	float Intelligence = 10.0f;

	// 体质 - 影响生命值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary Attributes")
	float Vitality = 10.0f;

	// 敏捷 - 影响闪避/暴击
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary Attributes")
	float Agility = 10.0f;

	/*
	 * 次要属性 (Secondary)
	 */

	// 护甲
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secondary Attributes")
	float Armor = 0.0f;

	// 暴击率 (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secondary Attributes", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float CriticalHitChance = 5.0f;

	// 暴击伤害倍数 (基础1.0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secondary Attributes", meta = (ClampMin = "1.0"))
	float CriticalHitDamage = 1.5f;

	// 生命恢复/秒
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secondary Attributes")
	float HealthRegeneration = 0.0f;

	// 法力恢复/秒
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secondary Attributes")
	float ManaRegeneration = 0.0f;

	/*
	 * 核心属性 (Vital)
	 */

	// 最大生命值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	// 最大怒气值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes", meta = (ClampMin = "1.0"))
	float MaxRage = 50.0f;

	// 最大法力值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes", meta = (ClampMin = "1.0"))
	float MaxMana = 80.0f;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes")
	float AttackPower = 10.0f;

	// 防御力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes")
	float DefensePower = 5.0f;
};

/**
 * 敌人类型枚举
 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Normal  UMETA(DisplayName = "普通敌人"),
	Elite   UMETA(DisplayName = "精英敌人"),
	Boss    UMETA(DisplayName = "Boss"),
	Minion  UMETA(DisplayName = "小怪")
};

/**
 * 敌人基础属性结构体
 * 与玩家属性对称设计，但针对敌人特点简化（无蓝量/怒气，增加抗性等）
 */
USTRUCT(BlueprintType)
struct FEnemyBaseAttributes
{
	GENERATED_BODY()

	// 核心资源（敌人通常只需要 HP）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vital Attributes")
	float MaxHealth = 100.0f;

	// 战斗属性
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Attributes")
	float AttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Attributes")
	float DefensePower = 5.0f;

	// 敌人特有属性 - 抗性系统（类魂游戏核心机制）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance Attributes")
	float Armor = 0.0f;                    // 护甲减伤

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance Attributes")
	float MagicResistance = 0.0f;          // 魔抗

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance Attributes")
	float StaggerResistance = 0.0f;        // 硬直抗性（类魂）

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance Attributes")
	float PoisonResistance = 0.0f;         // 毒抗

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance Attributes")
	float BleedResistance = 0.0f;          // 流血抗性

	// 掉落相关（非 GAS 属性，但在 DataAsset 中统一管理）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	int32 GoldDrop = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	int32 EXPDrop = 50;
};

USTRUCT(BlueprintType)
struct FRPGPlayerAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<URPGPlayerGameplayAbility> AbilityToGrant;

	bool IsValid() const;
	
};

USTRUCT(BlueprintType)
struct FRPGPlayerWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<URPGItemAnimLayersBase> WeaponAnimLayerToLink;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* EquipWeaponMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* UnequipWeaponMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* WeaponInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FRPGPlayerAbilitySet> DefaultWeaponAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FScalableFloat WeaponBaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SoftWeaponIconTexture;
};