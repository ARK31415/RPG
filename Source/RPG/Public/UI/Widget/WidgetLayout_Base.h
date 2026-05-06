// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "WidgetLayout_Base.generated.h"
/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNaiveTick))
class RPG_API UWidgetLayout_Base : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& InTag) const;

protected:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterWidgetStack(UPARAM(meta = (Categories = "RPGCommonUI.WidgetStack"))
	                         FGameplayTag InTag, UCommonActivatableWidgetContainerBase* InStack);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PrintAllWidgetsContainerTags();

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> RegisterWidgetStackMap;
};
