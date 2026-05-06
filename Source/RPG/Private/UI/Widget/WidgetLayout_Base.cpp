// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/WidgetLayout_Base.h"
#include "GameplayTagContainer.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGWidgetLayout_Base, All, All)

UCommonActivatableWidgetContainerBase* UWidgetLayout_Base::FindWidgetStackByTag(const FGameplayTag& InTag) const
{
	checkf(RegisterWidgetStackMap.Contains(InTag), TEXT("Tag %s Widget stack does not exist"), *InTag.ToString());
	return RegisterWidgetStackMap.FindRef(InTag);
}

void UWidgetLayout_Base::RegisterWidgetStack(UPARAM(meta = (Categories = "RPGCommonUI.WidgetStack")) FGameplayTag InTag, UCommonActivatableWidgetContainerBase* InStack)
{
	if (!IsDesignTime())
	{
		if (!RegisterWidgetStackMap.Contains(InTag))
		{
			RegisterWidgetStackMap.Add(InTag, InStack);
			UE_LOG(LogRPGWidgetLayout_Base, Warning, TEXT("Register Widget Stack: %s"), *InTag.ToString());
		}
	}
}

void UWidgetLayout_Base::PrintAllWidgetsContainerTags()
{
	TArray<FGameplayTag> AllTags;
	RegisterWidgetStackMap.GetKeys(AllTags);
	for (const FGameplayTag& Tag : AllTags)
	{
		UE_LOG(LogRPGWidgetLayout_Base, Warning, TEXT("Widget Container Tag: %s"), *Tag.ToString());
	}
}
