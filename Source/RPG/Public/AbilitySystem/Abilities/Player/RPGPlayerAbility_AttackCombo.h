#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"
#include "RPGPlayerAbility_AttackCombo.generated.h"

class UPlayerCombatComponent;

/**
 * Player Attack Combo Base Class
 * 
 * Core combo system for player attacks with input window support.
 * Inherited by URPGPlayerGameplayAbility to maintain type compatibility with FRPGPlayerAbilitySet.
 * 
 * Flow:
 * 1. ActivateAbility -> ResetComboState -> WaitForComboWindowEvents -> PlayCurrentComboMontage
 * 2. Input pressed during active combo -> sets bPendingComboInput flag
 * 3. AnimNotify sends ComboWindow.Open event -> checks bPendingComboInput -> advances combo
 * 4. Combo advances until MaxComboCount reached or window missed
 * 5. Montage completes -> EndAbility
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

	// Called when input is pressed while this ability is active
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void OnComboInputReceived();

	// Check if we can continue to next combo
	UFUNCTION(BlueprintPure, Category = "Combo")
	virtual bool CanContinueCombo() const;

	// Advance to next combo montage
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void AdvanceCombo();

	// Reset combo state (called on Activate and End)
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void ResetComboState();

	// Play current montage based on ComboIndex
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void PlayCurrentComboMontage();

	// Get PlayerCombatComponent from ActorInfo
	UFUNCTION(BlueprintPure, Category = "Combo")
	virtual UPlayerCombatComponent* GetCombatComponentFromActorInfo() const;

	// Combo window event handlers
	UFUNCTION()
	void OnComboWindowOpen(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboWindowClose(FGameplayEventData Payload);

	// Montage callbacks
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageBlendOut();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnMontageCancelled();

	// Wait for combo window events
	void WaitForComboWindowEvents();

protected:
	// Combo montages sequence (set in Blueprint subclass CDO)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<UAnimMontage*> ComboMontages;

	// Maximum combo count (e.g., 3 for 3-hit combo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	int32 MaxComboCount = 3;

	// Combo window duration in seconds (time allowed to input next attack)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float ComboWindowTime = 0.4f;

	// Current combo index (0-based)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo")
	int32 CurrentComboIndex = 0;

	// Flag: input was pressed during combo window
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo")
	bool bPendingComboInput = false;

	// Flag: combo window is currently open
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo")
	bool bIsInComboWindow = false;

	// Internal references
	TWeakObjectPtr<UPlayerCombatComponent> CachedCombatComponent;
};
