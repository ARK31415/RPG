// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "DataAsset/StartUpDate/DataAsset_PlayerStartUpData.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerState, All, All)

ARPGPlayerState::ARPGPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	RPGAbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("RPGAbilitySystemComponent"));
	RPGAbilitySystemComponent->SetIsReplicated(true);

	// Create attribute set
	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("RPGAttributeSet"));
}

UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return RPGAbilitySystemComponent;
}

void ARPGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Note: InitAbilityActorInfo is now called from Character side
	// (PossessedBy for server, OnRep_PlayerState for client)
	// to ensure Character mesh is available as AvatarActor

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
