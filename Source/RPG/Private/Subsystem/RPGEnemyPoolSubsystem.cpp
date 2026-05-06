// RPGEnemyPoolSubsystem.cpp - 敌人对象池实现

#include "Subsystem/RPGEnemyPoolSubsystem.h"
#include "Character/RPGEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/Health/RPGHealthComponent.h"

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
}

void URPGEnemyPoolSubsystem::ActivateEnemy(ARPGEnemyCharacter* Enemy, const FTransform& SpawnTransform)
{
	// 重置位置
	Enemy->SetActorTransform(SpawnTransform);

	// 显示并启用
	Enemy->SetActorHiddenInGame(false);
	Enemy->SetActorEnableCollision(true);
	Enemy->SetActorTickEnabled(true);

	// 恢复移动
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->SetComponentTickEnabled(true);
	}

	// 重新激活 ASC 和重置属性（由 BeginPlay 流程处理的内容需手动触发）
	// 注意：从池中复用的敌人需要在蓝图或调用处手动重新初始化
	// 例如：重新应用属性 GE、重启行为树等
	// 这里保持简单，让 AutoPossessAI 机制自动触发 PossessedBy

	// 触发 AI 重新接管
	Enemy->SpawnDefaultController();
}
