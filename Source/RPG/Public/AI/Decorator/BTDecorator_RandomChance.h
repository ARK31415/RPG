// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RandomChance.generated.h"

/**
 * 随机概率装饰器：以设定概率决定是否允许执行子节点
 */
UCLASS()
class RPG_API UBTDecorator_RandomChance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_RandomChance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** 通过概率 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random")
	float Probability;
};
