// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "DataAsset/StartUpDate/DataAsset_PlayerStartUpData.h"

ARPGPlayerState::ARPGPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	RPGAbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("RPGAbilitySystemComponent"));
	RPGAbilitySystemComponent->SetIsReplicated(true);

	// Create attribute set
	RPGAttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("RPGAttributeSet"));
}

UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return RPGAbilitySystemComponent;
}

void ARPGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ASC with avatar actor (the player character)
	if (RPGAbilitySystemComponent && GetPawn())
	{
		RPGAbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
	}

	// Initialize startup data (grant abilities and effects)
	InitializeStartupData();
}

void ARPGPlayerState::InitializeStartupData()
{
	if (!PlayerStartUpData || !RPGAbilitySystemComponent)
	{
		return;
	}

	// Grant abilities and effects from startup data
	PlayerStartUpData->GiveToAbilitySystemComponent(RPGAbilitySystemComponent, 1);
}
