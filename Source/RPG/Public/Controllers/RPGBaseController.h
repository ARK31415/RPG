// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "RPGBaseController.generated.h"


/**
 * 
 */
UCLASS()
class RPG_API ARPGBaseController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ARPGBaseController();
	
};
