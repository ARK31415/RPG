// Fill out your copyright notice in the Description page of Project Settings.


#include "RPGFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "RPGGameplayTags.h"
#include "Interface/PawnCombatInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGFunctionLibrary,All,All)

URPGAbilitySystemComponent* URPGFunctionLibrary::NativeGetRPGASCFromActor(AActor* InActor)
{
	// Return nullptr if InActor is invalid (supports editor preview)
	if (!InActor)
	{
		UE_LOG(LogRPGFunctionLibrary, Error, TEXT("InActor is nullptr") /*, InActor*/)
		return nullptr;
	}

	// Character and Enemy both implement IAbilitySystemInterface now
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);

	return Cast<URPGAbilitySystemComponent>(ASC);
}

void URPGFunctionLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	URPGAbilitySystemComponent* ASC = NativeGetRPGASCFromActor(InActor);
	if (!ASC)
	{
		UE_LOG(LogRPGFunctionLibrary, Error, TEXT("ASC is nullptr") /*, ASC*/)
		return;
	}

	if (!ASC->HasMatchingGameplayTag(TagToAdd))
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

void URPGFunctionLibrary::RemoveGameplayFromActorIfFound(AActor* InActor, FGameplayTag TagToRemove)
{
	URPGAbilitySystemComponent* ASC = NativeGetRPGASCFromActor(InActor);
	if (!ASC)
	{
		UE_LOG(LogRPGFunctionLibrary, Error, TEXT("ASC is nullptr") /*, ASC*/)
		return;
	}

	if (ASC->HasMatchingGameplayTag(TagToRemove))
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

bool URPGFunctionLibrary::NativeDoesActorHasTag(AActor* InActor, FGameplayTag TagToCheck)
{
	URPGAbilitySystemComponent* ASC = NativeGetRPGASCFromActor(InActor);
	if (!ASC)
	{
		UE_LOG(LogRPGFunctionLibrary, Error, TEXT("ASC is nullptr") /*, ASC*/)
		return false;
	}

	return ASC->HasMatchingGameplayTag(TagToCheck);
}

void URPGFunctionLibrary::BP_DoesActorHasTag(AActor* InActor, FGameplayTag TagToCheck, ERPGConfirmType& OutConfirmType)
{
	OutConfirmType = NativeDoesActorHasTag(InActor, TagToCheck) ? ERPGConfirmType::Yes : ERPGConfirmType::No;
}

UPawnCombatComponent* URPGFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
	// Use safe check instead of hard assertion to support editor preview
	if (!InActor)
	{
		UE_LOG(LogRPGFunctionLibrary, Error, TEXT("InActor is nullptr") /*, InActor*/)
		return nullptr;
	}

	if (IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}

	return nullptr;
}

UPawnCombatComponent* URPGFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor, ERPGValidType& OutValidType)
{
	UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponentFromActor(InActor);

	OutValidType = CombatComponent ? ERPGValidType::Valid : ERPGValidType::InValid;

	return CombatComponent;
}

bool URPGFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

	if (QueryTeamAgent && TargetTeamAgent)
	{
		return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}
	return false;
}

float URPGFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat, float InLevel)
{
	return InScalableFloat.GetValueAtLevel(InLevel);
}

FGameplayTag URPGFunctionLibrary::ComputeHitReactDirectionTag(AActor* InAttacker, AActor* InVictim,
                                                                  float& OutAngleDifference)
{
	check(InAttacker && InVictim);

	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

	const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);
	OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);

	// 使用叉积判断左右
	const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);

	if (OutAngleDifference <= 45.f)
	{
		// 正前方
		return RPGGameplayTags::Shared_Status_HitReact_Front;
	}
	else if (OutAngleDifference >= 135.f)
	{
		// 正后方
		return RPGGameplayTags::Shared_Status_HitReact_Back;
	}
	else if (CrossResult.Z > 0.f)
	{
		// 右侧
		return RPGGameplayTags::Shared_Status_HitReact_Right;
	}
	else
	{
		// 左侧
		return RPGGameplayTags::Shared_Status_HitReact_Left;
	}
}