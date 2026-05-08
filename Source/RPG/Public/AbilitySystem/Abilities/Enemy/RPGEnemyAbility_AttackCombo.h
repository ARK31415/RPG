#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGEnemyGameplayAbility.h"
#include "RPGEnemyAbility_AttackCombo.generated.h"

class UEnemyCombatComponent;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;
class URPGAttributeSet;

/**
 * Enemy Attack Combo Base Class
 * 
 * Core combo system for enemy attacks WITHOUT input window support.
 * Enemy combos are driven by AI logic, not player input.
 * 
 * Flow:
 * 1. ActivateAbility -> ResetComboState -> PlayCurrentComboMontage
 * 2. AI/BehaviorTree calls AdvanceCombo() to trigger next attack
 * 3. Combo advances until MaxComboCount reached
 * 4. Montage completes -> EndAbility
 */
UCLASS(Abstract, Blueprintable)
class RPG_API URPGEnemyAbility_AttackCombo : public URPGEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	URPGEnemyAbility_AttackCombo();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	// AI/BehaviorTree calls this to advance combo
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void AdvanceCombo();

	// Check if we can continue to next combo
	UFUNCTION(BlueprintPure, Category = "Combo")
	virtual bool CanContinueCombo() const;

	// Reset combo state (called on Activate and End)
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void ResetComboState();

	// Play current montage based on ComboIndex
	UFUNCTION(BlueprintCallable, Category = "Combo")
	virtual void PlayCurrentComboMontage();

	// Get EnemyCombatComponent from ActorInfo
	UFUNCTION(BlueprintPure, Category = "Combo")
	virtual UEnemyCombatComponent* GetCombatComponentFromActorInfo() const;

	// Montage callbacks
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageBlendOut();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnMontageCancelled();

	// Handle MeleeHit event and apply damage
	UFUNCTION()
	virtual void HandleApplyDamage(FGameplayEventData EventData);

	// Send HitReact event to target
	UFUNCTION()
	virtual void SendHitReactEvent(AActor* TargetActor, FGameplayEventData EventData);

protected:
	// Combo montages sequence (set in Blueprint subclass CDO)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<UAnimMontage*> ComboMontages;

	// Maximum combo count (e.g., 3 for 3-hit combo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	int32 MaxComboCount = 3;

	// Current combo index (0-based)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo")
	int32 CurrentComboIndex = 0;

	// Internal references
	TWeakObjectPtr<UEnemyCombatComponent> CachedCombatComponent;

	// Wait GameplayEvent task reference (for cleanup)
	UAbilityTask_WaitGameplayEvent* WaitMeleeHitEventTask = nullptr;

	// Damage GameplayEffect class (set in Blueprint or CDO)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
