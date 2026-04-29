// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Task/BTTask_FindRandomPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "NavigationSystem.h"
#include "NavigationData.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGBTTaskFindRandomPatrolPoint, Log, All)

UBTTask_FindRandomPatrolPoint::UBTTask_FindRandomPatrolPoint()
{
	NodeName = TEXT("Find Random Patrol Point");
	PatrolRadius = 1000.f;
}

EBTNodeResult::Type UBTTask_FindRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// 使用敌人当前位置作为巡逻中心点
	FVector CurrentLocation = Pawn->GetActorLocation();
	UE_LOG(LogRPGBTTaskFindRandomPatrolPoint, Log, TEXT("Searching for patrol point around %s, radius %.1f"), *CurrentLocation.ToString(), PatrolRadius);

	// 使用 NavigationSystem 获取随机可达点
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Pawn);
	if (!NavSystem)
	{
		UE_LOG(LogRPGBTTaskFindRandomPatrolPoint, Error, TEXT("NavigationSystem not found"));
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomLocation;
	bool bFound = NavSystem->GetRandomReachablePointInRadius(CurrentLocation, PatrolRadius, RandomLocation);

	if (bFound)
	{
		UBTFunctionLibrary::SetBlackboardValueAsVector(this, PatrolPointSelector, RandomLocation.Location);
		UE_LOG(LogRPGBTTaskFindRandomPatrolPoint, Log, TEXT("Found patrol point at %s"), *RandomLocation.Location.ToString());
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogRPGBTTaskFindRandomPatrolPoint, Warning, TEXT("Failed to find reachable patrol point in radius %.1f from %s"), PatrolRadius, *CurrentLocation.ToString());
	return EBTNodeResult::Failed;
}
