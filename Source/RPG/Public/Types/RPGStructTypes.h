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