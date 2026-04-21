// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RPGEnemyAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

/**
 * 敌人AI控制器
 * 管理行为树执行和Blackboard数据，驱动敌人AI决策
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

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlackboardComponent> BlackboardComp;
};
