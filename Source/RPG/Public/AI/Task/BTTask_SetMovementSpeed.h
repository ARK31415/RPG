// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_SetMovementSpeed.generated.h"

/**
 * 动态设置敌人移动速度的行为树任务节点
 */
UCLASS()
class RPG_API UBTTask_SetMovementSpeed : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_SetMovementSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 目标移动速度（从Blackboard读取） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FBlackboardKeySelector TargetSpeedSelector;
};
