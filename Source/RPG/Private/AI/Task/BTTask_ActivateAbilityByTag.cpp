// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_ActivateAbilityByTag.h"

#include "Character/RPGEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AIController.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGBTTaskActivateAbilityByTag, Log, All)
EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARPGEnemyCharacter* OwingEnemyCharacter = Cast<ARPGEnemyCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (OwingEnemyCharacter)
	{
		OwingEnemyCharacter->GetRPGAbilitySystemComponent()->TryActivateAbilityByTag(AbilityTag);
		UE_LOG(LogRPGBTTaskActivateAbilityByTag, Log, TEXT("Enemy: %s Ability activated by tag: %s"), *OwingEnemyCharacter->GetName(), *AbilityTag.ToString());
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
