// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GEExecCale/GEExecCale_DamageTaken.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAttributeSet.h"

//快方法
struct FRPGDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage)

	FRPGDamageCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, DamageTaken, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, CriticalHitDamage, Source, false);
	}
};

static const FRPGDamageCapture& GetRPGDamageCapture()
{
	static FRPGDamageCapture RPGDamageCapture;
	return RPGDamageCapture;
}

UGEExecCale_DamageTaken::UGEExecCale_DamageTaken()
{
	/*
	//慢方法
	FProperty* AttackPowerProperty = FindFieldChecked<FProperty>(
		URPGAttributeSet::StaticClass(),
		GET_MEMBER_NAME_CHECKED(URPGAttributeSet, AttackPower)
	);

	FGameplayEffectAttributeCaptureDefinition AttackPowerCaptureDefinition(
		AttackPowerProperty,
		EGameplayEffectAttributeCaptureSource::Source,
		false
	);

	RelevantAttributesToCapture.Add(AttackPowerCaptureDefinition);
	*/

	RelevantAttributesToCapture.Add(GetRPGDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetRPGDamageCapture().DefensePowerDef);
	RelevantAttributesToCapture.Add(GetRPGDamageCapture().DamageTakenDef);
	RelevantAttributesToCapture.Add(GetRPGDamageCapture().ArmorDef);
	RelevantAttributesToCapture.Add(GetRPGDamageCapture().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(GetRPGDamageCapture().CriticalHitDamageDef);
}

void UGEExecCale_DamageTaken::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetRPGDamageCapture().AttackPowerDef, EvaluateParameters, SourceAttackPower);
	//Debug::Print(TEXT("SourceAttackPower"), SourceAttackPower);
	
	float BaseDamage = 0.0f;
	int32 UsedLightAttackComboCount = 0;
	int32 UsedHeavyAttackComboCount = 0;
	
	for(const TPair<FGameplayTag, float>& TagMagnitude : EffectSpec.SetByCallerTagMagnitudes)
	{
		if(TagMagnitude.Key.MatchesTagExact(RPGGameplayTags::Shared_SetByCaller_BaseDamage))
		{
			BaseDamage = TagMagnitude.Value;
			//Debug::Print(TEXT("BaseDamage"), BaseDamage);
		}

		if(TagMagnitude.Key.MatchesTagExact(RPGGameplayTags::Player_SetByCaller_AttackType_Light))
		{
			UsedLightAttackComboCount = TagMagnitude.Value;
			//Debug::Print(TEXT("UsedLightAttackComboCount"), UsedLightAttackComboCount);
		}

		if(TagMagnitude.Key.MatchesTagExact(RPGGameplayTags::Player_SetByCaller_AttackType_Heavy))
		{
			UsedHeavyAttackComboCount = TagMagnitude.Value;
			//Debug::Print(TEXT("UsedHeavyAttackComboCount"), UsedHeavyAttackComboCount);
		}
	}
	
	float TargetDefensePower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetRPGDamageCapture().DefensePowerDef, EvaluateParameters, TargetDefensePower);

	float TargetArmor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetRPGDamageCapture().ArmorDef, EvaluateParameters, TargetArmor);

	float SourceCritChance = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetRPGDamageCapture().CriticalHitChanceDef, EvaluateParameters, SourceCritChance);

	float SourceCritDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetRPGDamageCapture().CriticalHitDamageDef, EvaluateParameters, SourceCritDamage);

	// 连击加成
	if(UsedLightAttackComboCount != 0)
	{
		const float DamageIncreasePercentLight = (UsedLightAttackComboCount - 1) * 0.05f + 1.f;
		BaseDamage *= DamageIncreasePercentLight;
	}

	if(UsedHeavyAttackComboCount != 0)
	{
		const float DamageIncreasePercentHeavy = UsedHeavyAttackComboCount * 0.15f + 1.f;
		BaseDamage *= DamageIncreasePercentHeavy;
	}

	// 暴击判定（CriticalHitChance 为 0-100 的百分比值）
	float CritMultiplier = 1.0f;
	if (SourceCritChance > 0.f)
	{
		const float RandRoll = FMath::FRandRange(0.f, 100.f);
		if (RandRoll <= SourceCritChance)
		{
			// CriticalHitDamage 表示额外倍率（如 0.5 = 150% 伤害）
			CritMultiplier = 1.0f + FMath::Max(0.f, SourceCritDamage);
		}
	}

	// 护甲减伤：每点护甲减少 1% 伤害，上限 80%
	const float ArmorReduction = FMath::Clamp(TargetArmor * 0.01f, 0.f, 0.8f);

	// 最终伤害 = 基础伤害 * 攻击力/防御力 * 暴击 * (1 - 护甲减伤)
	const float DefenseFactor = (TargetDefensePower > 0.f) ? (SourceAttackPower / TargetDefensePower) : 1.f;
	const float FinalDamageDone = BaseDamage * DefenseFactor * CritMultiplier * (1.f - ArmorReduction);
	
	if(FinalDamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				GetRPGDamageCapture().DamageTakenProperty,
				EGameplayModOp::Override,
				FinalDamageDone
			)	
		);
	}
}
