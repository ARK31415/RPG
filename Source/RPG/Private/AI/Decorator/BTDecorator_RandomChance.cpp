// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Decorator/BTDecorator_RandomChance.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogBTDecoratorRandomChance, Log, All)

UBTDecorator_RandomChance::UBTDecorator_RandomChance()
{
	NodeName = TEXT("Random Chance");
	Probability = 0.5f; // 默认50%概率
}

bool UBTDecorator_RandomChance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// 生成0~1随机数，小于概率则通过
	float RandomValue = UKismetMathLibrary::RandomFloat();
	bool bResult = RandomValue <= Probability;

	UE_LOG(LogBTDecoratorRandomChance, Log, TEXT("Random Chance: %.2f (threshold: %.2f) - %s"),
		RandomValue, Probability, bResult ? TEXT("PASS") : TEXT("FAIL"));

	return bResult;
}
