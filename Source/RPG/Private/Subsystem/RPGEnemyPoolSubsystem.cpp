// RPGEnemyPoolSubsystem.cpp - 敌人对象池实现

#include "Subsystem/RPGEnemyPoolSubsystem.h"

#include "RPGGameplayTags.h"
#include "Character/RPGEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/Health/RPGHealthComponent.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyPool, Log, All)

bool URPGEnemyPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 仅在 Game World 中创建（编辑器预览不需要）
	UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void URPGEnemyPoolSubsystem::Deinitialize()
{
	// 清理池中所有 Actor
	for (auto& Pair : Pool)
	{
		for (ARPGEnemyCharacter* Enemy : Pair.Value)
		{
			if (IsValid(Enemy))
			{
				Enemy->Destroy();
			}
		}
	}
	Pool.Empty();

	Super::Deinitialize();
}

ARPGEnemyCharacter* URPGEnemyPoolSubsystem::AcquireEnemy(TSubclassOf<ARPGEnemyCharacter> EnemyClass, const FTransform& SpawnTransform)
{
	if (!EnemyClass)
	{
		UE_LOG(LogRPGEnemyPool, Warning, TEXT("AcquireEnemy: EnemyClass is null"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 尝试从池中取出
	TArray<TObjectPtr<ARPGEnemyCharacter>>* PoolArray = Pool.Find(EnemyClass.Get());
	if (PoolArray && PoolArray->Num() > 0)
	{
		ARPGEnemyCharacter* Enemy = PoolArray->Pop();
		if (IsValid(Enemy))
		{
			ActivateEnemy(Enemy, SpawnTransform);
			UE_LOG(LogRPGEnemyPool, Log, TEXT("AcquireEnemy: Reused [%s] from pool (remaining: %d)"), 
				*Enemy->GetName(), PoolArray->Num());
			return Enemy;
		}
	}

	// 池中无可用实例，新建
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARPGEnemyCharacter* NewEnemy = World->SpawnActor<ARPGEnemyCharacter>(EnemyClass, SpawnTransform, SpawnParams);
	if (NewEnemy)
	{
		UE_LOG(LogRPGEnemyPool, Log, TEXT("AcquireEnemy: Spawned new [%s] (no pooled instance available)"), *NewEnemy->GetName());
	}

	return NewEnemy;
}

void URPGEnemyPoolSubsystem::ReleaseEnemy(ARPGEnemyCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}

	DeactivateEnemy(Enemy);

	UClass* EnemyClass = Enemy->GetClass();
	TArray<TObjectPtr<ARPGEnemyCharacter>>& PoolArray = Pool.FindOrAdd(EnemyClass);
	PoolArray.Add(Enemy);

	UE_LOG(LogRPGEnemyPool, Log, TEXT("ReleaseEnemy: [%s] returned to pool (pool size: %d)"), 
		*Enemy->GetName(), PoolArray.Num());
}

void URPGEnemyPoolSubsystem::WarmPool(TSubclassOf<ARPGEnemyCharacter> EnemyClass, int32 Count)
{
	if (!EnemyClass || Count <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 在场景外的位置生成，然后立即 Deactivate
	const FTransform HiddenTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, -10000.f));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TArray<TObjectPtr<ARPGEnemyCharacter>>& PoolArray = Pool.FindOrAdd(EnemyClass.Get());

	for (int32 i = 0; i < Count; ++i)
	{
		ARPGEnemyCharacter* Enemy = World->SpawnActor<ARPGEnemyCharacter>(EnemyClass, HiddenTransform, SpawnParams);
		if (Enemy)
		{
			DeactivateEnemy(Enemy);
			PoolArray.Add(Enemy);
		}
	}

	UE_LOG(LogRPGEnemyPool, Log, TEXT("WarmPool: Pre-spawned %d instances of [%s]"), Count, *EnemyClass->GetName());
}

int32 URPGEnemyPoolSubsystem::GetAvailableCount(TSubclassOf<ARPGEnemyCharacter> EnemyClass) const
{
	if (!EnemyClass)
	{
		return 0;
	}

	const TArray<TObjectPtr<ARPGEnemyCharacter>>* PoolArray = Pool.Find(EnemyClass.Get());
	return PoolArray ? PoolArray->Num() : 0;
}

void URPGEnemyPoolSubsystem::DeactivateEnemy(ARPGEnemyCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}

	// 隐藏并禁用
	Enemy->SetActorHiddenInGame(true);
	Enemy->SetActorEnableCollision(false);
	Enemy->SetActorTickEnabled(false);

	// 禁用移动
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetComponentTickEnabled(false);
	}

	// 取消 LifeSpan（阻止自动销毁）
	Enemy->SetLifeSpan(0.f);

	// 停止 AI
	if (AController* Controller = Enemy->GetController())
	{
		Controller->UnPossess();
	}

	// 清理 ASC 状态
	if (URPGAbilitySystemComponent* ASC = Enemy->GetRPGAbilitySystemComponent())
	{
		// 清除所有 Active GameplayEffects
		ASC->RemoveActiveEffectsWithAppliedTags(FGameplayTagContainer());
		
		// 清除所有 Loose GameplayTags（需要传入空容器来清除所有）
		FGameplayTagContainer AllTags;
		ASC->GetOwnedGameplayTags(AllTags);
		ASC->RemoveLooseGameplayTags(AllTags);
		
		UE_LOG(LogRPGEnemyPool, Log, TEXT("DeactivateEnemy: [%s] ASC cleared"), *Enemy->GetName());
	}

	// 重置健康组件死亡状态
	if (URPGHealthComponent* HealthComp = Enemy->FindComponentByClass<URPGHealthComponent>())
	{
		// 重置死亡标记（通过重新初始化或手动重置）
		// 注意：HealthComponent 的内部状态会在下次 ActivateEnemy 时通过属性重置来恢复
	}

	// 禁用战斗组件
	if (UEnemyCombatComponent* CombatComp = Enemy->GetEnemyCombatComponent())
	{
		CombatComp->SetComponentTickEnabled(false);
	}

	// 将敌人移至隐藏位置
	const FVector PoolStagingLocation(0.f, 0.f, -10000.f);
	Enemy->SetActorLocation(PoolStagingLocation);
	
	UE_LOG(LogRPGEnemyPool, Log, TEXT("DeactivateEnemy: [%s] deactivated and staged"), *Enemy->GetName());
}

void URPGEnemyPoolSubsystem::ActivateEnemy(ARPGEnemyCharacter* Enemy, const FTransform& SpawnTransform)
{
	if (!IsValid(Enemy))
	{
		return;
	}

	// 重置位置
	Enemy->SetActorTransform(SpawnTransform);

	// 显示并启用
	Enemy->SetActorHiddenInGame(false);
	Enemy->SetActorEnableCollision(true);
	Enemy->SetActorTickEnabled(true);

	// 恢复移动
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetComponentTickEnabled(true);
	}

	// 完整重置 ASC 状态
	if (URPGAbilitySystemComponent* ASC = Enemy->GetRPGAbilitySystemComponent())
	{
		// 重新初始化 ActorInfo
		ASC->InitAbilityActorInfo(Enemy, Enemy);
		
		// 清除所有 Active GameplayEffects（保留Granted Abilities）
		ASC->RemoveActiveEffectsWithAppliedTags(FGameplayTagContainer());
		
		// 清除所有 Loose GameplayTags
		FGameplayTagContainer AllTags;
		ASC->GetOwnedGameplayTags(AllTags);
		ASC->RemoveLooseGameplayTags(AllTags);
		
		// 重置健康组件状态
		if (URPGHealthComponent* HealthComp = Enemy->FindComponentByClass<URPGHealthComponent>())
		{
			// 直接设置属性值来重置生命值
			const URPGAttributeSet* AttributeSet = ASC->GetSet<URPGAttributeSet>();
			if (AttributeSet)
			{
				const float MaxHealth = AttributeSet->GetMaxHealth();
				ASC->SetNumericAttributeBase(URPGAttributeSet::GetCurrentHealthAttribute(), MaxHealth);
				UE_LOG(LogRPGEnemyPool, Log, TEXT("ActivateEnemy: [%s] Health reset to %.0f"), *Enemy->GetName(), MaxHealth);
			}
		}
	}

	// 重置战斗组件
	if (UEnemyCombatComponent* CombatComp = Enemy->GetEnemyCombatComponent())
	{
		CombatComp->SetComponentTickEnabled(true);
	}

	// 触发 AI 重新接管
	Enemy->SpawnDefaultController();

	// 通知 Character 完成激活后的统一初始化（由 Character 协调所有组件）
	Enemy->OnEnemyActivated();
	
	UE_LOG(LogRPGEnemyPool, Log, TEXT("ActivateEnemy: [%s] activated at %s"), 
		*Enemy->GetName(), *SpawnTransform.GetLocation().ToString());
}
