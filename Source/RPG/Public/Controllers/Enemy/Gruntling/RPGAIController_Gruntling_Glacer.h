// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/RPGEnemyAIController.h"
#include "RPGAIController_Gruntling_Glacer.generated.h"

UCLASS()
class RPG_API ARPGAIController_Gruntling_Glacer : public ARPGEnemyAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARPGAIController_Gruntling_Glacer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
