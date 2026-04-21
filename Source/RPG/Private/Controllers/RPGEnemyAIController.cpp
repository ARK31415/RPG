// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/RPGEnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyAIController, All, All)

ARPGEnemyAIController::ARPGEnemyAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 行为树驱动，不需要每帧Tick
	PrimaryActorTick.bCanEverTick = false;

	// 创建BehaviorTree和Blackboard组件
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// 设置Blackboard为AIController的默认Blackboard
	Blackboard = BlackboardComp;
}

void ARPGEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ARPGEnemyAIController::RunBehaviorTreeWithBlackboard(UBehaviorTree* InBehaviorTree)
{
	if (!InBehaviorTree)
	{
		UE_LOG(LogEnemyAIController, Warning, TEXT("[%s] RunBehaviorTreeWithBlackboard - BehaviorTree 为空"), *GetName());
		return;
	}

	// 初始化Blackboard
	if (InBehaviorTree->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*InBehaviorTree->BlackboardAsset);
	}

	// 运行行为树
	RunBehaviorTree(InBehaviorTree);

	UE_LOG(LogEnemyAIController, Log, TEXT("[%s] RunBehaviorTreeWithBlackboard - 行为树[%s]已启动"),
		*GetName(), *InBehaviorTree->GetName());
}

