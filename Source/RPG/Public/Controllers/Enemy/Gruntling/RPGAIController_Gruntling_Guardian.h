// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/RPGEnemyAIController.h"
#include "RPGAIController_Gruntling_Guardian.generated.h"

UCLASS()
class RPG_API ARPGAIController_Gruntling_Guardian : public ARPGEnemyAIController
{
	GENERATED_BODY()

public:
	ARPGAIController_Gruntling_Guardian();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void Tick(float DeltaTime) override;
};
