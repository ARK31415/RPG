// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimationInstances/RPGItemAnimLayersBase.h"

#include "Component/Combat/PlayerCombatComponent.h"
#include "GameFramework/Character.h"

URPGItemAnimLayersBase::URPGItemAnimLayersBase()
	: WeaponType(ERPGWeaponType::None)
	, CombatState(ERPGCombatState::Idle)
	, ComboIndex(0)
	, MaxComboCount(0)
	, bIsInComboWindow(false)
	, AttackSpeedMultiplier(1.0f)
	, bIsAttacking(false)
{
}

void URPGItemAnimLayersBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (OwningCharacter)
	{
		CombatComponent = OwningCharacter->FindComponentByClass<UPlayerCombatComponent>();
	}
}

void URPGItemAnimLayersBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	SyncFromCombatComponent();
}

void URPGItemAnimLayersBase::SetWeaponType(ERPGWeaponType NewType)
{
	WeaponType = NewType;
}

void URPGItemAnimLayersBase::SyncFromCombatComponent()
{
	// 连招状态已迁移至GAS Ability层
	// 动画层只需通过Montage播放接收表现指令
	// 未来如需同步状态，可通过GameplayTag或事件机制
}
