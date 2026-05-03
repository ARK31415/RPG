// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/Enemy/Gruntling/RPGAIController_Gruntling_Guardian.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGAIController_Gruntling_Guardian, All, All)
// Sets default values
ARPGAIController_Gruntling_Guardian::ARPGAIController_Gruntling_Guardian(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Tick已在父类 ARPGEnemyAIController 中关闭（行为树驱动）
	// 感知系统已在父类构造函数中初始化
}

// Called when the game starts or when spawned
void ARPGAIController_Gruntling_Guardian::BeginPlay()
{
	Super::BeginPlay();

	// Guardian 特有的感知参数配置
	SightRadius = 2000.f;
	LoseSightRadius = 2500.f;
	PeripheralVisionAngle = 120.f;
	PerceptionMaxAge = 3.f;
	bDetectEnemies = true;

	// 重新配置感知组件（因为参数已修改）
	if (EnemyPerceptionComponent && EnemySightConfig)
	{
		EnemySightConfig->SightRadius = SightRadius;
		EnemySightConfig->LoseSightRadius = LoseSightRadius;
		EnemySightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
		EnemySightConfig->SetMaxAge(PerceptionMaxAge);
		EnemySightConfig->DetectionByAffiliation.bDetectEnemies = bDetectEnemies;

		EnemyPerceptionComponent->ConfigureSense(*EnemySightConfig);
	}
}

// AI控制器接管Pawn时调用（在行为树启动后）
void ARPGAIController_Gruntling_Guardian::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

// 感知到目标时的自定义处理（重写父类虚函数）
void ARPGAIController_Gruntling_Guardian::OnPerceptionTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	Super::OnPerceptionTargetDetected(Actor, Stimulus);

	/*
	if (!Actor || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	// 如果感知到玩家（假设玩家有特定Tag或类型）
	if (Actor->ActorHasTag(FName("Player")))
	{
		UE_LOG(LogRPGAIController_Gruntling_Guardian, Log, TEXT("[Guardian] Detected Player: %s"), *Actor->GetName());
		// 这里可以触发追逐行为、播放音效等
	}*/
}

// Called every frame
void ARPGAIController_Gruntling_Guardian::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

