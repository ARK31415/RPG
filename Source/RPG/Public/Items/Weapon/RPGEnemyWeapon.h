// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGWeaponBase.h"
#include "RPGEnemyWeapon.generated.h"

UCLASS()
class RPG_API ARPGEnemyWeapon : public ARPGWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARPGEnemyWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
