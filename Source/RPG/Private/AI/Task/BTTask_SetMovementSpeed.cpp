// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Task/BTTask_SetMovementSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBTTaskSetMovementSpeed, Log, All)

UBTTask_SetMovementSpeed::UBTTask_SetMovementSpeed()
{
	NodeName = TEXT("Set Movement Speed");
}

EBTNodeResult::Type UBTTask_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return EBTNodeResult::Failed;
	}

	// 从 Blackboard 读取目标速度
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	float TargetSpeed = Blackboard->GetValueAsFloat(TargetSpeedSelector.SelectedKeyName);

	MovementComp->MaxWalkSpeed = TargetSpeed;

	UE_LOG(LogBTTaskSetMovementSpeed, Log, TEXT("Set movement speed to %.1f for %s"), TargetSpeed, *Character->GetName());
	return EBTNodeResult::Succeeded;
}
