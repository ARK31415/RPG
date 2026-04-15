// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/RPGEnumTypes.h"
#include "Types/RPGStructTypes.h"
#include "DataAsset_CharacterConfig.generated.h"

class URPGAbilitySystemComponent;

/**
 * 角色配置数据资产
 * 定义角色基础信息和初始属性（黑魂模式：武器决定战斗行为，这里只配置角色基础属性）
 */
UCLASS(BlueprintType)
class RPG_API UDataAsset_CharacterConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UDataAsset_CharacterConfig();

	/**
	 * 将配置的基础属性通过 GameplayEffect 应用到 ASC
	 * @param InASC 目标 AbilitySystemComponent
	 * @param Level 角色等级
	 */
	void ApplyAttributesToASC(URPGAbilitySystemComponent* InASC, int32 Level = 1) const;

	// 基础信息
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	FName CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	ERPGCharacterClass CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	FText CharacterDescription;

	// 角色基础属性配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Attributes")
	FCharacterBaseAttributes BaseAttributes;
};
