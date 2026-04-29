// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "RPGEnemyAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * 敌人AI控制器
 * 管理行为树执行和Blackboard数据，驱动敌人AI决策
 * 包含AI感知系统（视觉感知），所有敌人共享
 */
UCLASS()
class RPG_API ARPGEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARPGEnemyAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 运行指定的行为树（由敌人Character在PossessedBy中调用） */
	void RunBehaviorTreeWithBlackboard(UBehaviorTree* InBehaviorTree);

	/** 获取BehaviorTree组件 */
	UFUNCTION(BlueprintPure, Category = "AI")
	UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return BehaviorTreeComponent; }

protected:
	virtual void BeginPlay() override;

	// ========== AI Perception 配置（子类可覆盖） ==========

	/** 视觉感知半径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float SightRadius;

	/** 丢失视觉半径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float LoseSightRadius;

	/** 边缘视野角度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float PeripheralVisionAngle;

	/** 感知信息有效期 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float PerceptionMaxAge;

	/** 是否检测敌人（玩家） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	bool bDetectEnemies;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlackboardComponent> BlackboardComp;

	// AI感知组件
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComponent;

	// 视觉感知配置
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> EnemySightConfig;

	// 缓存已感知的目标
	TMap<AActor*, float> PerceivedActors;

	// 初始化感知系统（在构造函数中调用）
	void InitializePerception();

	// 感知更新回调
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 更新最近目标到Blackboard
	void UpdateNearestTarget();

	// 虚函数：允许子类自定义感知后的行为
	virtual void OnPerceptionTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};
