// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "DataAsset/StartUpDate/DataAsset_EnemyStartUpData.h"
#include "DataAsset/Character/DataAsset_EnemyConfig.h"

ARPGEnemyCharacter::ARPGEnemyCharacter()
{
	// Create ability system component on the character itself (for enemies)
	RPGAbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("RPGAbilitySystemComponent"));
	RPGAbilitySystemComponent->SetIsReplicated(true);

	// Create attribute set
	RPGAttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("RPGAttributeSet"));
}

UAbilitySystemComponent* ARPGEnemyCharacter::GetAbilitySystemComponent() const
{
	return RPGAbilitySystemComponent;
}

UEnemyCombatComponent* ARPGEnemyCharacter::GetEnemyCombatComponent() const
{
	return EnemyCombatComponent;
}

void ARPGEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ASC with avatar actor (the enemy character itself)
	if (RPGAbilitySystemComponent)
	{
		RPGAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 初始化敌人配置（属性）
	InitializeEnemyConfig();

	// Initialize startup data (grant abilities and effects)
	InitializeStartupData();
}

void ARPGEnemyCharacter::InitializeStartupData()
{
	if (!EnemyStartUpData || !RPGAbilitySystemComponent)
	{
		return;
	}

	// Grant abilities and effects from startup data
	EnemyStartUpData->GiveToAbilitySystemComponent(RPGAbilitySystemComponent, 1);
}

void ARPGEnemyCharacter::InitializeEnemyConfig()
{
	if (!EnemyConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeEnemyConfig - EnemyConfig 为空，无法初始化敌人属性"), *GetName());
		return;
	}

	if (!RPGAbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] InitializeEnemyConfig - ASC 为空"), *GetName());
		return;
	}

	// 应用敌人属性到 ASC
	EnemyConfig->ApplyAttributesToASC(RPGAbilitySystemComponent, 1);

	UE_LOG(LogTemp, Log, TEXT("[%s] InitializeEnemyConfig - 敌人属性已应用到 ASC, Config=[%s]"),
		*GetName(), *EnemyConfig->GetName());
}
