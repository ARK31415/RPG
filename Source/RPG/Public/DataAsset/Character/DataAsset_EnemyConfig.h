// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/RPGStructTypes.h"
#include "DataAsset_EnemyConfig.generated.h"

class URPGAbilitySystemComponent;

/**
 * 敌人配置数据资产
 * 与 Player 的 UDataAsset_CharacterConfig 对称设计
 * 定义敌人基础信息和初始属性（独立于 StartupData）
 */
UCLASS(BlueprintType)
class RPG_API UDataAsset_EnemyConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UDataAsset_EnemyConfig();

	/**
	 * 将配置的敌人属性通过 GameplayEffect 应用到 ASC
	 * @param InASC 目标 AbilitySystemComponent
	 * @param Level 敌人等级（用于后续等级缩放）
	 */
	void ApplyAttributesToASC(URPGAbilitySystemComponent* InASC, int32 Level = 1) const;

	// 敌人基础信息
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Info")
	FName EnemyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Info")
	EEnemyType EnemyType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Info")
	FText EnemyDescription;

	// 敌人基础属性配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Attributes")
	FEnemyBaseAttributes BaseAttributes;
};
