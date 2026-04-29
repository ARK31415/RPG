// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_FindRandomPatrolPoint.generated.h"

/**
 * 在出生点附近随机半径内寻找一个可达的巡逻点，写入Blackboard
 */
UCLASS()
class RPG_API UBTTask_FindRandomPatrolPoint : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_FindRandomPatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 巡逻半径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	float PatrolRadius;

	/** 输出：巡逻点位置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	FBlackboardKeySelector PatrolPointSelector;
};
