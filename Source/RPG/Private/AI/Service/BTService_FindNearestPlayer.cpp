// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_FindNearestPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGBTServiceFindNearestPlayer, Log, All)

UBTService_FindNearestPlayer::UBTService_FindNearestPlayer()
{
	bNotifyTick = true;
	Interval = 0.5f;  // 每0.5秒执行一次
	RandomDeviation = 0.0f;
}

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* OwningPawn = AIOwner->GetPawn();

	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");
	//UE_LOG(LogRPGBTServiceFindNearestPlayer, Log, TEXT("Target Tag: %s"), *TargetTag.ToString())

	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);

	/*for (AActor* Actor : ActorsWithTag)
	{
		UE_LOG(LogRPGBTServiceFindNearestPlayer, Log, TEXT("Actor with tag: %s"), *Actor->GetName())
	}*/

	float ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;
	for (AActor* Actor : ActorsWithTag)
	{
		if (IsValid(Actor) && IsValid(OwningPawn))
		{
			const float Distance = OwningPawn->GetDistanceTo(Actor);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
	}
	UBTFunctionLibrary::SetBlackboardValueAsObject(this,TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
