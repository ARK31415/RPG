#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "Types/RPGEnumTypes.h"
#include "RPGPlayerAbility_AttackCombo.generated.h"

class UPlayerCombatComponent;

/**
 * Player Attack Combo Base Class
 * 
 * Core combo system for player attacks with timer-based combo window.
 * 
 * Flow:
 * 1. Player attacks -> ActivateAbility -> PlayCurrentComboMontage -> Start reset timer
 * 2. If player attacks again within ComboWindowTime -> AdvanceComboCount -> Play next montage
 * 3. If timer expires -> ResetComboCount -> Back to first combo
 */
UCLASS(Abstract, Blueprintable)
class RPG_API URPGPlayerAbility_AttackCombo : public URPGPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	URPGPlayerAbility_AttackCombo();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Play current montage based on ComboIndex
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void PlayCurrentComboMontage();

	// Advance combo to next hit (called when player inputs attack during combo window)
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void AdvanceComboCount();

	// Get PlayerCombatComponent from ActorInfo
	UFUNCTION(BlueprintPure, Category = "Combo")
	virtual UPlayerCombatComponent* GetCombatComponentFromActorInfo() const;

	// Montage callbacks
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageBlendOut();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnMontageCancelled();

	// Reset combo count to 1
	UFUNCTION()
	virtual void ResetComboCount();

	// Start combo window timer (called when ability ends)
	UFUNCTION()
	virtual void StartComboWindowTimer();

protected:
	// Combo montages sequence (set in Blueprint subclass CDO)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TMap<int32, UAnimMontage*> ComboMontages;

	// Maximum combo count (e.g., 3 for 3-hit combo)
	UPROPERTY()
	int32 MaxComboCount = 0;

	// Combo window duration in seconds (time allowed to input next attack)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float ComboWindowTime = 3.f;

	// Current combo type (set by subclass: LightAttack or HeavyAttack)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	ERPGComboType ComboType = ERPGComboType::LightAttack;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo")
	int32 CurrentLightAttackComboCount = 1;

	// Internal references
	TWeakObjectPtr<UPlayerCombatComponent> CachedCombatComponent;
};
