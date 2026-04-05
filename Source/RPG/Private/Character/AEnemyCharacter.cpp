// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "DataAsset/StartUpDate/DataAsset_EnemyStartUpData.h"

AEnemyCharacter::AEnemyCharacter()
{
	// Create ability system component on the character itself (for enemies)
	RPGAbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("RPGAbilitySystemComponent"));
	RPGAbilitySystemComponent->SetIsReplicated(true);

	// Create attribute set
	RPGAttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("RPGAttributeSet"));
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return RPGAbilitySystemComponent;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ASC with avatar actor (the enemy character itself)
	if (RPGAbilitySystemComponent)
	{
		RPGAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// Initialize startup data (grant abilities and effects)
	InitializeStartupData();
}

void AEnemyCharacter::InitializeStartupData()
{
	if (!EnemyStartUpData || !RPGAbilitySystemComponent)
	{
		return;
	}

	// Grant abilities and effects from startup data
	EnemyStartUpData->GiveToAbilitySystemComponent(RPGAbilitySystemComponent, 1);
}
