// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_FindStrafingPoint_EQS.generated.h"

class UEnvQuery;

/**
 * 使用EQS查询玩家周围的可移动环绕点，写入Blackboard
 */
UCLASS()
class RPG_API UBTTask_FindStrafingPoint_EQS : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_FindStrafingPoint_EQS();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** EQS查询资产 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EQS")
	TObjectPtr<UEnvQuery> StrafingQuery;

	/** 输出：环绕点位置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EQS")
	FBlackboardKeySelector StrafingPointSelector;

	/** 上下文：目标玩家（从Blackboard读取） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EQS")
	FBlackboardKeySelector TargetSelector;
};
