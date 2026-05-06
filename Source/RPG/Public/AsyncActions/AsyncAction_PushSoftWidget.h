// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UI/Widget/RPGWidget_ActivatableBase.h"
#include "AsyncAction_PushSoftWidget.generated.h"

class URPGWidget_ActivatableBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushSoftWidgetDelegate, URPGWidget_ActivatableBase*, PushedWidget);

/**
 * 
 */
UCLASS()
class RPG_API UAsyncAction_PushSoftWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject",
		BlueprintInternalUseOnly = true, DisplayName = "Push Soft Widget To Stack"))
	static UAsyncAction_PushSoftWidget* PushSoftWidget(const UObject* WorldContextObject,
	                                                   APlayerController* OwningPlayerController,
	                                                   TSoftClassPtr<URPGWidget_ActivatableBase> InSoftWidgetClass,
	                                                   UPARAM(meta = (Categories = "RPGCommonUI.WidgetStack"))
	                                                   FGameplayTag InWidgetStackTag,
	                                                   bool bFocusOnNewlyPushWidget = true);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnWidgetCreatedBeforePush;

	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnAfterPush;

private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;
	TWeakObjectPtr<APlayerController> CachedOwningPlayerController;
	TSoftClassPtr<URPGWidget_ActivatableBase> CachedSoftWidgetClass;
	FGameplayTag CachedWidgetStackTag;
	bool bCachedFocusOnNewlyPushWidget = false;
};
