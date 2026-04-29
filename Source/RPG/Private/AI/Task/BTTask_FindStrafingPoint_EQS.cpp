// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Task/BTTask_FindStrafingPoint_EQS.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogBTTaskFindStrafingPointEQS, Log, All)

UBTTask_FindStrafingPoint_EQS::UBTTask_FindStrafingPoint_EQS()
{
	NodeName = TEXT("Find Strafing Point (EQS)");
}

EBTNodeResult::Type UBTTask_FindStrafingPoint_EQS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 此节点需要在蓝图中重载实现 EQS 查询逻辑
	// C++ 端只提供框架，实际 EQS 查询在蓝图子类中完成
	UE_LOG(LogBTTaskFindStrafingPointEQS, Warning, TEXT("BTTask_FindStrafingPoint_EQS: 请在蓝图中重载此节点并实现 EQS 查询逻辑"));
	
	// 默认返回成功，避免阻塞行为树（实际逻辑应在蓝图中实现）
	return EBTNodeResult::Succeeded;
}
