// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "RPGEnemyCharacter.generated.h"

class URPGAbilitySystemComponent;
class URPGAttributeSet;
class UDataAsset_EnemyStartUpData;

/**
 * Enemy Character Base Class - 用于敌人的ASC和属性管理
 * 简化AI敌人的实现，敌人不需要持久化数据
 */
UCLASS()
class RPG_API ARPGEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ARPGEnemyCharacter();

	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Getter functions for ASC and AttributeSet
	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const { return RPGAbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "RPG|AbilitySystem")
	URPGAttributeSet* GetRPGAttributeSet() const { return RPGAttributeSet; }

protected:
	virtual void BeginPlay() override;

private:
	// Ability System Component for enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|AbilitySystem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;

	// Attribute Set for enemy stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|AbilitySystem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URPGAttributeSet> RPGAttributeSet;

	// Startup data to grant abilities and effects on spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Startup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_EnemyStartUpData> EnemyStartUpData;

	// Initialize startup data (grant abilities and effects)
	void InitializeStartupData();
};
