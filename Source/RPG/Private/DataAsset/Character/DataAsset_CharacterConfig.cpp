// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Character/DataAsset_CharacterConfig.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogCharacterConfig, All, All)

UDataAsset_CharacterConfig::UDataAsset_CharacterConfig()
	: CharacterClass(ERPGCharacterClass::None)
{
}

void UDataAsset_CharacterConfig::ApplyAttributesToASC(URPGAbilitySystemComponent* InASC, int32 Level) const
{
	if (!InASC)
	{
		UE_LOG(LogCharacterConfig, Error, TEXT("[%s] ApplyAttributesToASC - ASC 为空，无法应用属性"), *GetName());
		return;
	}

	// 创建一个临时的 Instant GameplayEffect 来初始化属性
	UGameplayEffect* InitEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_InitCharacterAttributes")));
	InitEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	// === 主属性 ===
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetStrengthAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.Strength));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetIntelligenceAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.Intelligence));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetVitalityAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.Vitality));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetAgilityAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.Agility));
		InitEffect->Modifiers.Add(Mod);
	}

	// === 次要属性 ===
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetArmorAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.Armor));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetCriticalHitChanceAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.CriticalHitChance));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetCriticalHitDamageAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.CriticalHitDamage));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetHealthRegenerationAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.HealthRegeneration));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetManaRegenerationAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.ManaRegeneration));
		InitEffect->Modifiers.Add(Mod);
	}

	// === 核心属性 ===
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetMaxHealthAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.MaxHealth));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetMaxRageAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.MaxRage));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetMaxManaAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.MaxMana));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetAttackPowerAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.AttackPower));
		InitEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetDefensePowerAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.DefensePower));
		InitEffect->Modifiers.Add(Mod);
	}

	// 应用基础属性 GE
	FGameplayEffectContextHandle EffectContext = InASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	InASC->ApplyGameplayEffectToSelf(InitEffect, Level, EffectContext);

	// 第二个 GE：设置当前值（满血/满蓝状态，怒气初始为0）
	UGameplayEffect* FillEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_FillVitalAttributes")));
	FillEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetCurrentHealthAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.MaxHealth));
		FillEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetCurrentRageAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.0f)); // 怒气初始为0
		FillEffect->Modifiers.Add(Mod);
	}
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = URPGAttributeSet::GetCurrentManaAttribute();
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(BaseAttributes.MaxMana));
		FillEffect->Modifiers.Add(Mod);
	}

	InASC->ApplyGameplayEffectToSelf(FillEffect, Level, EffectContext);

	// 打印属性初始化日志
	UE_LOG(LogCharacterConfig, Log,
		TEXT("[%s] ApplyAttributesToASC - 角色[%s] 属性初始化完成 | ")
		TEXT("Str=%.1f Int=%.1f Vit=%.1f Agi=%.1f | ")
		TEXT("MaxHP=%.1f MaxRage=%.1f MaxMana=%.1f | ")
		TEXT("ATK=%.1f DEF=%.1f"),
		*GetName(),
		*CharacterName.ToString(),
		BaseAttributes.Strength, BaseAttributes.Intelligence, BaseAttributes.Vitality, BaseAttributes.Agility,
		BaseAttributes.MaxHealth, BaseAttributes.MaxRage, BaseAttributes.MaxMana,
		BaseAttributes.AttackPower, BaseAttributes.DefensePower
	);
}
