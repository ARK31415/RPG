// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGWeaponBase.generated.h"

class UBoxComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeaponHitTargetDelegate, ARPGWeaponBase* /*Weapon*/, AActor* /*HitActor*/)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeaponPulledFromTargetDelegate, ARPGWeaponBase* /*Weapon*/, AActor* /*TargetActor*/)

UCLASS()
class RPG_API ARPGWeaponBase : public AActor
{
	GENERATED_BODY()
public:	
	
	ARPGWeaponBase();

	/** 武器命中目标事件（多播，支持多个监听者：战斗组件/音效/特效等） */
	FOnWeaponHitTargetDelegate OnWeaponHitTarget;
	
	/** 武器拔出目标事件 */
	FOnWeaponPulledFromTargetDelegate OnWeaponPulledFromTarget;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UBoxComponent* WeaponCollisionBox;

	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const {return WeaponCollisionBox;}

};
