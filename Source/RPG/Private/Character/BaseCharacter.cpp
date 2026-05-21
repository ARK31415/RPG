// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "MotionWarpingComponent.h"
#include "RPGDebugHelper.h"
// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//tick不能关，否则Player左右移动会有问题
	PrimaryActorTick.bCanEverTick = true;
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPawnCombatComponent* ABaseCharacter::GetPawnCombatComponent() const
{
	return nullptr;
}

void ABaseCharacter::OnDeathStarted_Implementation()
{
	// 默认空实现，由子类重写（玩家/敌人有不同的死亡逻辑）
	Debug::Log(TEXT("[BaseCharacter] OnDeathStarted - Default implementation, should be overridden"));
}

void ABaseCharacter::OnDeathFinished_Implementation()
{
	// 默认空实现，由子类重写（玩家/敌人有不同的死亡逻辑）
	Debug::Log(TEXT("[BaseCharacter] OnDeathFinished - Default implementation, should be overridden"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::InitAbilityActorInfo()
{
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

