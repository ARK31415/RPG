// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/UI/RPGPlayerUIComponent.h"
#include "Component/Health/RPGPlayerHealthComponent.h"
#include "Character/RPGPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerUIComponent, Log, All)

URPGPlayerUIComponent::URPGPlayerUIComponent()
{
	CurrentMana = 100.0f;
	MaxMana = 100.0f;
}

URPGHealthComponent* URPGPlayerUIComponent::GetHealthComponentInternal() const
{
	// 从 Player Character 获取 PlayerHealthComponent
	if (const ARPGPlayerCharacter* PlayerCharacter = Cast<ARPGPlayerCharacter>(GetOwner()))
	{
		return PlayerCharacter->GetHealthComponent();
	}
	return nullptr;
}

void URPGPlayerUIComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化 Mana 系统
	InitializeManaSystem();

	UE_LOG(LogRPGPlayerUIComponent, Log, TEXT("PlayerUIComponent initialized for %s"), *GetOwner()->GetName());
}

void URPGPlayerUIComponent::InitializeManaSystem()
{
	// TODO: 从 AttributeSet 获取 Mana 属性
	// 这里先使用默认值
	
	UE_LOG(LogRPGPlayerUIComponent, Log, TEXT("Mana system initialized: %.0f/%.0f"), 
		CurrentMana, MaxMana);
	
	// 广播初始 Mana 值
	OnManaChangedForUI.Broadcast(CurrentMana, 0.0f);
}
