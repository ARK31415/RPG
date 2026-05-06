// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RPGWidget_ActivatableBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNaiveTick))
class RPG_API URPGWidget_ActivatableBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
};
