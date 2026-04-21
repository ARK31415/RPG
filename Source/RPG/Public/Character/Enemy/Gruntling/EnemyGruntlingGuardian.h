// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyGruntlingBase.h"
#include "EnemyGruntlingGuardian.generated.h"

UCLASS()
class RPG_API AEnemyGruntlingGuardian : public AEnemyGruntlingBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyGruntlingGuardian();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
