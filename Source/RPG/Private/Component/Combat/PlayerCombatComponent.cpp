// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/PlayerCombatComponent.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerCombatComponent, All, All)

UPlayerCombatComponent::UPlayerCombatComponent()
{
	InitComboCounts();
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	InitComboCounts();
}

void UPlayerCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		for (auto& Pair : ComboResetTimers)
		{
			GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
		}
		ComboResetTimers.Empty();
	}
	Super::EndPlay(EndPlayReason);
}

void UPlayerCombatComponent::InitComboCounts()
{
	ComboCounts.FindOrAdd(ERPGComboType::LightAttack) = 1;
	ComboCounts.FindOrAdd(ERPGComboType::HeavyAttack) = 1;
}

int32 UPlayerCombatComponent::GetComboCount(ERPGComboType ComboType) const
{
	const int32* Count = ComboCounts.Find(ComboType);
	return Count ? *Count : 1;
}

void UPlayerCombatComponent::SetComboCount(ERPGComboType ComboType, int32 NewCount)
{
	ComboCounts.FindOrAdd(ComboType) = NewCount;
}

void UPlayerCombatComponent::ResetComboCount(ERPGComboType ComboType)
{
	ComboCounts.FindOrAdd(ComboType) = 1;
	UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Combo count reset to 1 for type: %d"), static_cast<uint8>(ComboType));
}

void UPlayerCombatComponent::AdvanceComboCount(ERPGComboType ComboType, int32 MaxComboCount)
{
	int32& CurrentCount = ComboCounts.FindOrAdd(ComboType);
	if (CurrentCount >= MaxComboCount)
	{
		CurrentCount = 1;
		UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Max combo reached for type %d, resetting to 1"), static_cast<uint8>(ComboType));
	}
	else
	{
		CurrentCount++;
		UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Combo count advanced to %d for type %d"), CurrentCount, static_cast<uint8>(ComboType));
	}
}

void UPlayerCombatComponent::SwitchComboType(ERPGComboType NewComboType)
{
	// 切换攻击类型时重置对方计数器
	for (auto& Pair : ComboCounts)
	{
		if (Pair.Key != NewComboType)
		{
			UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] SwitchComboType: Resetting type %d combo count (was %d)"), static_cast<uint8>(Pair.Key), Pair.Value);
			Pair.Value = 1;

			// 清除对方的定时器
			if (FTimerHandle* Timer = ComboResetTimers.Find(Pair.Key))
			{
				if (GetWorld())
				{
					GetWorld()->GetTimerManager().ClearTimer(*Timer);
				}
			}
		}
	}
}

void UPlayerCombatComponent::StartComboWindowTimer(ERPGComboType ComboType, float WindowTime)
{
	if (!GetWorld())
	{
		return;
	}

	// 清除该类型之前的定时器
	FTimerHandle& TimerHandle = ComboResetTimers.FindOrAdd(ComboType);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	// 设置新的定时器
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UPlayerCombatComponent::OnComboWindowTimerExpired, ComboType);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		TimerDelegate,
		WindowTime,
		false
	);

	UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Combo window timer started for type %d (%.2f seconds)"), static_cast<uint8>(ComboType), WindowTime);
}

void UPlayerCombatComponent::SetCurrentComboType(ERPGComboType Type)
{
	CurrentComboType = Type;
	UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] Set CurrentComboType to %d"), static_cast<uint8>(Type));
}

void UPlayerCombatComponent::OnComboWindowTimerExpired(ERPGComboType ComboType)
{
	UE_LOG(LogRPGPlayerCombatComponent, Warning, TEXT("[PlayerCombatComponent] >>> Combo window timer expired for type %d, resetting combo <<<"), static_cast<uint8>(ComboType));
	ResetComboCount(ComboType);
}

ARPGPlayerWeapon* UPlayerCombatComponent::GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
	return Cast<ARPGPlayerWeapon>(GetCharacterCarriedWeaponByTag(InWeaponTag)); 
}

ARPGPlayerWeapon* UPlayerCombatComponent::GetPlayerCurrentEquippedWeapon() const
{
	return Cast<ARPGPlayerWeapon>(GetCharacterCurrentEquippedWeapon());
}

float UPlayerCombatComponent::GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLeveL) const
{
	if (ARPGPlayerWeapon* Weapon = GetPlayerCurrentEquippedWeapon())
	{
		return Weapon->PlayerWeaponData.WeaponBaseDamage.GetValueAtLevel(InLeveL);
	}
	return 0.f;
}

void UPlayerCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	if(OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);

	FGameplayEventData Data;
	Data.Instigator = GetOwningPawn();
	Data.Target = HitActor;

	UE_LOG(LogRPGPlayerCombatComponent, Log, TEXT("[PlayerCombatComponent] OnHitTargetActor: %s"), *HitActor->GetName());

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Shared_Event_MeleeHit,
		Data
	);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Player_Event_HitPause,
		FGameplayEventData()
	);
}

void UPlayerCombatComponent::OnWeaponPullerFromTargetActor(AActor* InteractedActor)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		RPGGameplayTags::Player_Event_HitPause,
		FGameplayEventData()
	);
}
