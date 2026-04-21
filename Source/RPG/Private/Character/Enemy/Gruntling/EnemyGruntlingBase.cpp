// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Gruntling/EnemyGruntlingBase.h"


// Sets default values
AEnemyGruntlingBase::AEnemyGruntlingBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemyGruntlingBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyGruntlingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyGruntlingBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

