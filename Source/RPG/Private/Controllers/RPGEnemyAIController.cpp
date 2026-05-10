// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/RPGEnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyAIController, All, All)

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

	// 感知系统默认配置
	SightRadius = 1500.f;
	LoseSightRadius = 2000.f;
	PeripheralVisionAngle = 90.f;
	PerceptionMaxAge = 5.f;
	bDetectEnemies = true;

	// 初始化感知系统
	InitializePerception();

	SetGenericTeamId(FGenericTeamId(1));
	//UE_LOG(LogRPGEnemyAIController, Warning, TEXT("Enemies GenericTeamId: %d"), GetGenericTeamId().GetId());
}

ETeamAttitude::Type ARPGEnemyAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<const APawn>(&Other);
	if (!PawnToCheck)
	{
		// 非 Pawn 类型，默认友好
		return ETeamAttitude::Friendly;
	}

	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(PawnToCheck->GetController());

	if (OtherTeamAgent)
	{
		// 有队伍系统，进行敌我判断
		if (OtherTeamAgent->GetGenericTeamId() < GetGenericTeamId())
		{
			UE_LOG(LogRPGEnemyAIController, Warning, TEXT("[%s] GetTeamAttitudeTowards OtherTeamId %d < SelfTeamId %d - Hostile"), 
				*GetName(), OtherTeamAgent->GetGenericTeamId().GetId(), GetGenericTeamId().GetId());
			return ETeamAttitude::Hostile;
		}
		else
		{
			UE_LOG(LogRPGEnemyAIController, Warning, TEXT("[%s] GetTeamAttitudeTowards OtherTeamId %d >= SelfTeamId %d - Friendly"), 
				*GetName(), OtherTeamAgent->GetGenericTeamId().GetId(), GetGenericTeamId().GetId());
			return ETeamAttitude::Friendly;
		}
	}
	
	// 没有队伍系统（无 Controller 或未实现接口），默认中立/友好
	UE_LOG(LogRPGEnemyAIController, Warning, TEXT("[%s] GetTeamAttitudeTowards - No team agent, default Friendly"), *GetName());
	return ETeamAttitude::Friendly;
}

void ARPGEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	// 🔹 步骤1：启用 Sight Sense（.qoder reference: "网络同步基础.md"）
	if (EnemyPerceptionComponent)
	{
		EnemyPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	}
	
}
void ARPGEnemyAIController::RunBehaviorTreeWithBlackboard(UBehaviorTree* InBehaviorTree)
{
	if (!InBehaviorTree)
	{
		UE_LOG(LogRPGEnemyAIController, Warning, TEXT("[%s] RunBehaviorTreeWithBlackboard - BehaviorTree 为空"), *GetName());
		return;
	}

	// 初始化Blackboard
	if (InBehaviorTree->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*InBehaviorTree->BlackboardAsset);
	}

	// 运行行为树
	RunBehaviorTree(InBehaviorTree);

	UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] RunBehaviorTreeWithBlackboard - 行为树[%s]已启动"),
		*GetName(), *InBehaviorTree->GetName());
}

// 初始化感知系统
void ARPGEnemyAIController::InitializePerception()
{
	// 创建视觉感知配置
	EnemySightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("EnemySightConfig"));
	EnemySightConfig->SightRadius = SightRadius;
	EnemySightConfig->LoseSightRadius = LoseSightRadius;
	EnemySightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
	EnemySightConfig->SetMaxAge(PerceptionMaxAge);
	EnemySightConfig->DetectionByAffiliation.bDetectEnemies = true;
	EnemySightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	EnemySightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	// 创建感知组件
	EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
	EnemyPerceptionComponent->ConfigureSense(*EnemySightConfig);
	EnemyPerceptionComponent->SetDominantSense(EnemySightConfig->GetSenseImplementation());
	EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnEnemyPerceptionUpdated);
}

// 感知更新回调
void ARPGEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

	UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] OnTargetPerceptionUpdated - Actor: %s"), *GetName(), *Actor->GetName());
	if (!Actor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// 感知到目标，添加到缓存
		float Distance = GetPawn() ? GetPawn()->GetDistanceTo(Actor) : 0.f;
		PerceivedActors.Add(Actor, Distance);
		UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] Perceived target: %s at distance %.1f"), *GetName(), *Actor->GetName(), Distance);
	}
	else
	{
		// 丢失目标，从缓存移除
		PerceivedActors.Remove(Actor);
		UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] Lost target: %s"), *GetName(), *Actor->GetName());
	}

	// 触发子类自定义行为
	OnPerceptionTargetDetected(Actor, Stimulus);

	// 更新最近目标到Blackboard
	UpdateNearestTarget();
}

void ARPGEnemyAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] OnEnemyPerceptionUpdated - Actor: %s"), *GetName(), *Actor->GetName());
	if(UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		if(!BlackboardComponent->GetValueAsObject(FName("TargetActor")))
		{
			if(Stimulus.WasSuccessfullySensed() && Actor)
			{
				BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
			}
		}
	}
}

// 更新最近目标到Blackboard
void ARPGEnemyAIController::UpdateNearestTarget()
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (!BBComp)
	{
		return;
	}

	if (PerceivedActors.Num() == 0)
	{
		// 没有感知到目标，清空Blackboard
		BBComp->SetValueAsObject(FName("TargetActor"), nullptr);
		BBComp->SetValueAsFloat(FName("DistanceToTarget"), 0.f);
		BBComp->SetValueAsBool(FName("HasTarget"), false);
		UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] No targets perceived, cleared Blackboard"), *GetName());
		return;
	}

	// 找到距离最近的目标
	AActor* NearestActor = nullptr;
	float MinDistance = TNumericLimits<float>::Max();

	for (const TPair<AActor*, float>& Pair : PerceivedActors)
	{
		if (Pair.Key && Pair.Value < MinDistance)
		{
			MinDistance = Pair.Value;
			NearestActor = Pair.Key;
		}
	}

	if (NearestActor)
	{
		BBComp->SetValueAsObject(FName("TargetActor"), NearestActor);
		BBComp->SetValueAsFloat(FName("DistanceToTarget"), MinDistance);
		BBComp->SetValueAsBool(FName("HasTarget"), true);
		UE_LOG(LogRPGEnemyAIController, Log, TEXT("[%s] Updated nearest target: %s at distance %.1f"), *GetName(), *NearestActor->GetName(), MinDistance);
	}
}

// 虚函数：子类可重载自定义行为
void ARPGEnemyAIController::OnPerceptionTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 默认实现为空，子类可重载
}

