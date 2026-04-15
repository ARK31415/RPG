// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RPGAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


URPGAttributeSet::URPGAttributeSet()
{
	/*
	 * 主属性初始化
	 * 这些值应该通过 GameplayEffect 和 DataAsset 来配置，这里只是默认值
	 */
	InitStrength(1.0f);
	InitIntelligence(1.0f);
	InitVitality(1.0f);
	InitAgility(1.0f);

	/*
	 * 次要属性初始化
	 */
	InitArmor(0.0f);
	InitCriticalHitChance(0.0f);
	InitCriticalHitDamage(1.0f); // 基础1.0表示100%伤害
	InitHealthRegeneration(0.0f);
	InitManaRegeneration(0.0f);

	/*
	 * 核心属性初始化
	 * 这些值通常由主属性通过 GameplayEffect 计算得出
	 */
	InitCurrentHealth(1.0f);
	InitMaxHealth(1.0f);
	InitCurrentRage(0.0f);
	InitMaxRage(1.0f);
	InitCurrentMana(0.0f);
	InitMaxMana(1.0f);

	/*
	 * 元属性初始化
	 */
	InitDamageTaken(0.0f);
	InitIncomingXP(0.0f);
	InitAttackPower(1.0f); // 保留兼容性
	InitDefensePower(1.0f); // 保留兼容性
}

void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 主属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Vitality, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Agility, COND_None, REPNOTIFY_Always);

	// 次要属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);

	// 核心属性 - 这些是关键属性，需要快速同步
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentRage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxRage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	// 元属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, DefensePower, COND_None, REPNOTIFY_Always);

	// DamageTaken 和 IncomingXP 不需要同步，它们只是临时计算用的
}

void URPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 在属性改变之前进行验证和限制
	Super::PreAttributeChange(Attribute, NewValue);

	// 核心属性验证 - 确保值在合理范围内
	if (Attribute == GetCurrentHealthAttribute())
	{
		// 生命值不能为负，不能超过最大值
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetCurrentRageAttribute())
	{
		// 怒气值不能为负，不能超过最大值
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxRage());
	}
	else if (Attribute == GetCurrentManaAttribute())
	{
		// 法力值不能为负，不能超过最大值
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxHealthAttribute() || 
			 Attribute == GetMaxRageAttribute() || 
			 Attribute == GetMaxManaAttribute())
	{
		// 最大值必须大于0
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetCriticalHitChanceAttribute())
	{
		// 暴击率限制在 0-100 之间
		NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
	else if (Attribute == GetCriticalHitDamageAttribute())
	{
		// 暴击伤害至少为 1.0 (100%)
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void URPGAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 当最大值改变时，调整当前值以确保不超过新的最大值
	if (Attribute == GetMaxHealthAttribute())
	{
		if (GetCurrentHealth() > NewValue)
		{
			SetCurrentHealth(NewValue);
		}
	}
	else if (Attribute == GetMaxRageAttribute())
	{
		if (GetCurrentRage() > NewValue)
		{
			SetCurrentRage(NewValue);
		}
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		if (GetCurrentMana() > NewValue)
		{
			SetCurrentMana(NewValue);
		}
	}

	// 注意：这里可以添加属性变更后的其他逻辑
	// 例如：触发UI更新、播放特效等
}

void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 处理伤害计算
	if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		const float OldHealth = GetCurrentHealth();
		const float DamageDone = GetDamageTaken();
		const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.0f, GetMaxHealth());
		
		SetCurrentHealth(NewCurrentHealth);
		SetDamageTaken(0.0f); // 重置临时属性

		// 死亡检测
		if (NewCurrentHealth <= 0.0f)
		{
			// 注意：这里应该通过事件或标签系统处理死亡逻辑
			// 而不是直接引用游戏标签，保持解耦
		}
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		// 直接修改生命值时确保在有效范围内
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.0f, GetMaxRage());
		SetCurrentRage(NewCurrentRage);
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentManaAttribute())
	{
		const float NewCurrentMana = FMath::Clamp(GetCurrentMana(), 0.0f, GetMaxMana());
		SetCurrentMana(NewCurrentMana);
	}
}

/*
 * OnRep 函数实现 - 网络同步后的回调
 * 这些函数在客户端接收到服务器同步的属性值后调用
 */

// 主属性 OnRep
void URPGAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Strength, OldStrength);
}

void URPGAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Intelligence, OldIntelligence);
}

void URPGAttributeSet::OnRep_Vitality(const FGameplayAttributeData& OldVitality) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Vitality, OldVitality);
}

void URPGAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Agility, OldAgility);
}

// 次要属性 OnRep
void URPGAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Armor, OldArmor);
}

void URPGAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void URPGAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void URPGAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void URPGAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, ManaRegeneration, OldManaRegeneration);
}

// 核心属性 OnRep - 这些是最重要的，通常需要更新UI
void URPGAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CurrentHealth, OldCurrentHealth);
	// TODO: 触发UI更新事件
	// 例如：BroadcastCurrentHealthChanged(GetCurrentHealth() / GetMaxHealth());
}

void URPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxHealth, OldMaxHealth);
	// TODO: 触发UI更新事件
}

void URPGAttributeSet::OnRep_CurrentRage(const FGameplayAttributeData& OldCurrentRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CurrentRage, OldCurrentRage);
	// TODO: 触发UI更新事件
}

void URPGAttributeSet::OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxRage, OldMaxRage);
	// TODO: 触发UI更新事件
}

void URPGAttributeSet::OnRep_CurrentMana(const FGameplayAttributeData& OldCurrentMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CurrentMana, OldCurrentMana);
	// TODO: 触发UI更新事件
}

void URPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxMana, OldMaxMana);
	// TODO: 触发UI更新事件
}

// 元属性 OnRep
void URPGAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, AttackPower, OldAttackPower);
}

void URPGAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, DefensePower, OldDefensePower);
}