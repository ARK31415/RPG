// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Character/DataAsset_EnemyConfig.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "GameplayEffect.h"

UDataAsset_EnemyConfig::UDataAsset_EnemyConfig()
{
	// 默认值可在子类或蓝图中覆盖
}

void UDataAsset_EnemyConfig::ApplyAttributesToASC(URPGAbilitySystemComponent* InASC, int32 Level) const
{
	if (!InASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ApplyAttributesToASC - ASC 为空，无法应用属性"), *EnemyName.ToString());
		return;
	}

	// ==========================
	// GE 1: 初始化所有基础属性
	// ==========================
	UGameplayEffect* GEFx = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_InitEnemyAttributes"));
	GEFx->DurationPolicy = EGameplayEffectDurationType::Instant;

	int32 Idx = 0;

	auto AddModifier = [GEFx, &Idx](FGameplayAttribute Attribute, float Value)
	{
		FGameplayModifierInfo ModInfo;
		ModInfo.Attribute = Attribute;
		ModInfo.ModifierMagnitude = FScalableFloat(Value);
		ModInfo.ModifierOp = EGameplayModOp::Override;
		GEFx->Modifiers.Add(ModInfo);
	};

	// 核心资源
	AddModifier(URPGAttributeSet::GetMaxHealthAttribute(), BaseAttributes.MaxHealth);

	// 战斗属性
	AddModifier(URPGAttributeSet::GetAttackPowerAttribute(), BaseAttributes.AttackPower);
	AddModifier(URPGAttributeSet::GetDefensePowerAttribute(), BaseAttributes.DefensePower);

	// 护甲（复用玩家的 Armor 属性）
	AddModifier(URPGAttributeSet::GetArmorAttribute(), BaseAttributes.Armor);

	UE_LOG(LogTemp, Display, TEXT("[%s] ApplyAttributesToASC - 敌人[%s] 属性初始化完成 | HP=%.1f ATK=%.1f DEF=%.1f Armor=%.1f"),
		*EnemyName.ToString(),
		*UEnum::GetValueAsString(EnemyType),
		BaseAttributes.MaxHealth,
		BaseAttributes.AttackPower,
		BaseAttributes.DefensePower,
		BaseAttributes.Armor);

	InASC->ApplyGameplayEffectToSelf(GEFx, Level, InASC->MakeEffectContext());

	// ==========================
	// GE 2: 填充当前生命值
	// ==========================
	UGameplayEffect* GEFill = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_FillEnemyHealth"));
	GEFill->DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo FillModInfo;
	FillModInfo.Attribute = URPGAttributeSet::GetCurrentHealthAttribute();
	FillModInfo.ModifierMagnitude = FScalableFloat(BaseAttributes.MaxHealth); // 满血生成
	FillModInfo.ModifierOp = EGameplayModOp::Override;
	GEFill->Modifiers.Add(FillModInfo);

	InASC->ApplyGameplayEffectToSelf(GEFill, Level, InASC->MakeEffectContext());
}
