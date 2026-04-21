// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/Enemy/Gruntling/RPGAIController_Gruntling_Guardian.h"


// Sets default values
ARPGAIController_Gruntling_Guardian::ARPGAIController_Gruntling_Guardian()
{
	// Tick已在父类 ARPGEnemyAIController 中关闭（行为树驱动）
}

// Called when the game starts or when spawned
void ARPGAIController_Gruntling_Guardian::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARPGAIController_Gruntling_Guardian::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

