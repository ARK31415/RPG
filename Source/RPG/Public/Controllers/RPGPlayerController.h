// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGBaseController.h"
#include "RPGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API ARPGPlayerController : public ARPGBaseController
{
	GENERATED_BODY()
public:
	ARPGPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	virtual FGenericTeamId GetGenericTeamId() const override;

	void EnsureGameInputMode();

private:
	FGenericTeamId PlayerTeamId;
	FTimerHandle InputModeTimerHandle;
};
