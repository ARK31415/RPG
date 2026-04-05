// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/RPGEnumTypes.h"
#include "DataAsset_CharacterConfig.generated.h"

/**
 * 角色配置数据资产 - 仅定义角色基础信息（黑魂模式：武器决定战斗行为）
 */
UCLASS(BlueprintType)
class RPG_API UDataAsset_CharacterConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UDataAsset_CharacterConfig();

	// 基础信息
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	FName CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	ERPGCharacterClass CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Info")
	FText CharacterDescription;
};
