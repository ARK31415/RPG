// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interface/PawnCombatInterface.h"
#include "Interface/PawnUIInterface.h"
#include "BaseCharacter.generated.h"

class URPGAttributeSet;
class UAttributeSet;
class URPGHealthComponent;
class URPGPlayerUIComponent;
class URPGEnemyUIComponent;

UCLASS()
class RPG_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface, public IPawnCombatInterface, public IPawnUIInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	
	// IPawnUIInterface implementation
	virtual URPGPlayerUIComponent* GetPlayerUIComponent() override { return nullptr; }
	virtual URPGEnemyUIComponent* GetEnemyUIComponent() override { return nullptr; }
	
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	/** 获取健康组件 */
	UFUNCTION(BlueprintPure, Category="Health")
	URPGHealthComponent* GetHealthComponent() const { return HealthComponent; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	TObjectPtr<URPGHealthComponent> HealthComponent;

	virtual void InitAbilityActorInfo();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
