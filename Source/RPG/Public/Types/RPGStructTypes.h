#pragma once

#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "Animation/AnimMontage.h"

#include "RPGStructTypes.generated.h"

class URPGItemAnimLayersBase;
class URPGPlayerGameplayAbility;
class URPGGameplayAbility;
class UInputMappingContext;

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

	// 战斗行为配置（黑魂模式：武器决定战斗风格）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Behavior")
	int32 MaxComboCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Behavior")
	float AttackSpeedMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Behavior")
	float BaseDamageMultiplier = 1.0f;
};