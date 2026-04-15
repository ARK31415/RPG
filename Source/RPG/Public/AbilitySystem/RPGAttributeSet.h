// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};


UCLASS()
class RPG_API URPGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	URPGAttributeSet();

	// 网络同步
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 属性变更回调
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/*
	 * 主属性 (Primary Attributes)
	 * 影响角色基础能力的核心属性
	 */
	
	// 力量 - 影响物理攻击力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Strength)

	// 智力 - 影响魔法攻击力/法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Intelligence)

	// 体质 - 影响生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vitality, Category = "Primary Attributes")
	FGameplayAttributeData Vitality;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Vitality)

	// 敏捷 - 影响闪避/暴击
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Primary Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Agility)

	/*
	 * 次要属性 (Secondary Attributes)
	 * 由主属性派生或直接配置的进阶属性
	 */

	// 护甲 - 减少物理伤害
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Armor)

	// 暴击率 - 暴击概率 (0-100)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, CriticalHitChance)

	// 暴击伤害 - 暴击时的伤害倍数 (基础1.0，表示100%伤害)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, CriticalHitDamage)

	// 生命恢复 - 每秒恢复的生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, HealthRegeneration)

	// 法力恢复 - 每秒恢复的法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, ManaRegeneration)

	/*
	 * 核心属性 (Vital Attributes)
	 * 角色存活和资源管理的关键属性
	 */

	// 当前生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Vital Attributes")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, CurrentHealth)

	// 最大生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxHealth)

	// 当前怒气值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentRage, Category = "Vital Attributes")
	FGameplayAttributeData CurrentRage;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, CurrentRage)

	// 最大怒气值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxRage, Category = "Vital Attributes")
	FGameplayAttributeData MaxRage;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxRage)

	// 当前法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentMana, Category = "Vital Attributes")
	FGameplayAttributeData CurrentMana;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, CurrentMana)

	// 最大法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxMana)

	/*
	 * 元属性 (Meta Attributes)
	 * 用于临时计算和事件传递的中间属性
	 */

	// 受到伤害 - 用于伤害计算
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData DamageTaken;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, DamageTaken)

	// 获得经验 - 用于经验值获取
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, IncomingXP)

	// 攻击力 (保留兼容性)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Meta Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, AttackPower)
	
	// 防御力 (保留兼容性)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DefensePower, Category = "Meta Attributes")
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, DefensePower)

	/*
	 * OnRep 函数声明 - 用于网络同步回调
	 */

	// 主属性 OnRep
	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	virtual void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	UFUNCTION()
	virtual void OnRep_Vitality(const FGameplayAttributeData& OldVitality) const;

	UFUNCTION()
	virtual void OnRep_Agility(const FGameplayAttributeData& OldAgility) const;

	// 次要属性 OnRep
	UFUNCTION()
	virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	UFUNCTION()
	virtual void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	UFUNCTION()
	virtual void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	UFUNCTION()
	virtual void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;

	UFUNCTION()
	virtual void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	// 核心属性 OnRep
	UFUNCTION()
	virtual void OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth) const;

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	virtual void OnRep_CurrentRage(const FGameplayAttributeData& OldCurrentRage) const;

	UFUNCTION()
	virtual void OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const;

	UFUNCTION()
	virtual void OnRep_CurrentMana(const FGameplayAttributeData& OldCurrentMana) const;

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	// 元属性 OnRep
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower) const;

	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower) const;

private:
	// 缓存的UI接口 (暂时注释)
	// TWeakInterfacePtr<IPawnUIInterface> CachedPawnUIInterface;
};
