// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner/RPGEnemySpawner.h"
#include "Character/RPGEnemyCharacter.h"
#include "Subsystem/RPGEnemyPoolSubsystem.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "RPGDebugHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemySpawner, Log, All)

ARPGEnemySpawner::ARPGEnemySpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));

	ArrowComponent = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->SetArrowColor(FLinearColor::Red);
	ArrowComponent->ArrowSize = 0.8f;

	SpawnAreaBox = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("SpawnArea"));
	SpawnAreaBox->SetupAttachment(RootComponent);
	SpawnAreaBox->SetBoxExtent(FVector(300.f, 300.f, 100.f));
	SpawnAreaBox->SetHiddenInGame(true);
	SpawnAreaBox->SetVisibility(false);
}

void ARPGEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoWarmOnBeginPlay && PreWarmCount > 0)
	{
		WarmPool();
	}

	// 自动化测试流程
	if (bEnableAutoTest)
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("========== [%s] Auto Test Started =========="), *GetName());
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Mode: %s"), SpawnMode == EEnemySpawnMode::Pool ? TEXT("Object Pool") : TEXT("Direct Spawn"));
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Test Spawn Count: %d"), TestSpawnCount);
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Delay After Warm: %.1fs"), TestDelayAfterWarm);
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Delay Before Release: %.1fs"), TestDelayBeforeRelease);

		// 重置统计
		ResetStats();

		// 第一步：等待热池后的延迟
		FTimerHandle WarmDelayHandle;
		GetWorldTimerManager().SetTimer(WarmDelayHandle, [this]()
		{
			UE_LOG(LogRPGEnemySpawner, Warning, TEXT("\n[%s] Step 1: Spawning %d enemies..."), *GetName(), TestSpawnCount);

			// 第二步：生成指定数量的敌人
			for (int32 i = 0; i < TestSpawnCount; ++i)
			{
				SpawnEnemy();
			}

			UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] Step 1 Complete: Spawned %d enemies, waiting %.1fs..."), 
				*GetName(), TestSpawnCount, TestDelayBeforeRelease);

			// 第三步：等待释放前的延迟
			FTimerHandle ReleaseDelayHandle;
			GetWorldTimerManager().SetTimer(ReleaseDelayHandle, [this]()
			{
				UE_LOG(LogRPGEnemySpawner, Warning, TEXT("\n[%s] Step 2: Releasing all enemies..."), *GetName());

				// 第四步：释放所有敌人
				ReleaseAllSpawnedEnemies();

				UE_LOG(LogRPGEnemySpawner, Warning, TEXT("\n[%s] Step 2 Complete: All enemies released"), *GetName());

				// 第五步：输出性能统计
				UE_LOG(LogRPGEnemySpawner, Warning, TEXT("\n"));
				LogStats();
				UE_LOG(LogRPGEnemySpawner, Warning, TEXT("========== [%s] Auto Test Finished ==========\n"), *GetName());

			}, TestDelayBeforeRelease, false);

		}, TestDelayAfterWarm, false);
	}
}

#if WITH_EDITOR
void ARPGEnemySpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(ARPGEnemySpawner, bUseBoxComponentAsSpawnArea))
		{
			if (SpawnAreaBox)
			{
				SpawnAreaBox->SetVisibility(bUseBoxComponentAsSpawnArea);
			}
		}
	}
}
#endif

URPGEnemyPoolSubsystem* ARPGEnemySpawner::GetEnemyPoolSubsystem() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<URPGEnemyPoolSubsystem>();
}

FTransform ARPGEnemySpawner::CalculateSpawnTransform() const
{
	if (bUseBoxComponentAsSpawnArea && SpawnAreaBox)
	{
		const FVector Origin = SpawnAreaBox->GetComponentLocation();
		const FVector Extent = SpawnAreaBox->GetScaledBoxExtent();
		const FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
		return FTransform(GetActorRotation(), RandomPoint);
	}

	const FVector BaseLocation = GetActorLocation() + GetActorRotation().RotateVector(SpawnOffset);
	const FVector RandomOffset = FVector(
		FMath::FRandRange(-SpawnSpreadRadius, SpawnSpreadRadius),
		FMath::FRandRange(-SpawnSpreadRadius, SpawnSpreadRadius),
		0.f
	);
	return FTransform(GetActorRotation(), BaseLocation + RandomOffset);
}

void ARPGEnemySpawner::WarmPool()
{
	if (!EnemyClass)
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] WarmPool - EnemyClass is null, skip"), *GetName());
		return;
	}

	URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
	if (!Pool)
	{
		UE_LOG(LogRPGEnemySpawner, Error, TEXT("[%s] WarmPool - EnemyPoolSubsystem not available"), *GetName());
		return;
	}

	Pool->WarmPool(EnemyClass, PreWarmCount);

	if (bShowDebug)
	{
		UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] WarmPool - Pre-warmed %d [%s], available: %d"),
			*GetName(), PreWarmCount, *EnemyClass->GetName(), Pool->GetAvailableCount(EnemyClass));
	}
}

ARPGEnemyCharacter* ARPGEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass)
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] SpawnEnemy - EnemyClass is null"), *GetName());
		return nullptr;
	}

	// 根据 SpawnMode 路由到不同的生成方法
	if (SpawnMode == EEnemySpawnMode::Direct)
	{
		return SpawnEnemyDirect();
	}

	// 对象池模式
	URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
	if (!Pool)
	{
		UE_LOG(LogRPGEnemySpawner, Error, TEXT("[%s] SpawnEnemy - EnemyPoolSubsystem not available, fallback to Direct"), *GetName());
		return SpawnEnemyDirect();  // 回退到普通生成
	}

	// 记录开始时间
	double StartTime = FPlatformTime::Seconds();
	UpdateMemoryStats();

	const FTransform SpawnTransform = CalculateSpawnTransform();
	ARPGEnemyCharacter* Enemy = Pool->AcquireEnemy(EnemyClass, SpawnTransform);

	// 记录耗时
	double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	UpdateMemoryStats();

	if (Enemy)
	{
		// 标记为对象池生成
		Enemy->Tags.AddUnique(FName("SpawnedFromPool"));
		PoolSpawnedEnemies.Add(Enemy);
		SpawnStats.TotalSpawnCount++;
		RecordSpawnStats(ElapsedMs, SpawnStats.CurrentMemoryBytes);

		if (bShowDebug)
		{
			const int32 Avail = Pool->GetAvailableCount(EnemyClass);
			UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] SpawnEnemy [Pool] - Spawned [%s] at %s, time: %.3fms, spawned total: %d, available: %d"),
				*GetName(), *Enemy->GetName(), *SpawnTransform.GetLocation().ToString(), ElapsedMs, PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num(), Avail);
		}
	}

	return Enemy;
}

TArray<ARPGEnemyCharacter*> ARPGEnemySpawner::SpawnMultipleEnemies(int32 Count)
{
	TArray<ARPGEnemyCharacter*> Result;
	Result.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		if (ARPGEnemyCharacter* Enemy = SpawnEnemy())
		{
			Result.Add(Enemy);
		}
	}

	return Result;
}

void ARPGEnemySpawner::ReleaseAllSpawnedEnemies()
{
	URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
	int32 ReleasedToPool = 0;
	int32 DestroyedDirect = 0;

	// 记录开始时间和内存
	double StartTime = FPlatformTime::Seconds();
	UpdateMemoryStats();

	// 回收对象池生成的敌人
	for (ARPGEnemyCharacter* Enemy : PoolSpawnedEnemies)
	{
		if (IsValid(Enemy) && Pool)
		{
			Pool->ReleaseEnemy(Enemy);
			++ReleasedToPool;
			++SpawnStats.TotalDestroyCount;
		}
	}

	// 销毁普通生成的敌人
	for (ARPGEnemyCharacter* Enemy : DirectSpawnedEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
			++DestroyedDirect;
			++SpawnStats.TotalDestroyCount;
		}
	}

	// 记录耗时
	double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	if (SpawnStats.TotalDestroyCount > 0)
	{
		RecordDestroyStats(ElapsedMs);
	}

	PoolSpawnedEnemies.Empty();
	DirectSpawnedEnemies.Empty();

	if (bShowDebug)
	{
		UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] ReleaseAllSpawnedEnemies - Released %d to pool, Destroyed %d direct, time: %.3fms"),
			*GetName(), ReleasedToPool, DestroyedDirect, ElapsedMs);
	}
}

void ARPGEnemySpawner::ReleaseSpawnedEnemy(ARPGEnemyCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] ReleaseSpawnedEnemy - Enemy is invalid"), *GetName());
		return;
	}

	// 查找该敌人的生成模式
	bool bFromPool = false;
	if (PoolSpawnedEnemies.Contains(Enemy))
	{
		bFromPool = true;
		PoolSpawnedEnemies.Remove(Enemy);
	}
	else if (DirectSpawnedEnemies.Contains(Enemy))
	{
		bFromPool = false;
		DirectSpawnedEnemies.Remove(Enemy);
	}
	else
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] ReleaseSpawnedEnemy - Enemy [%s] not found in spawned list"), *GetName(), *Enemy->GetName());
		return;
	}

	// 根据生成模式决定回收方式
	if (bFromPool)
	{
		// 对象池生成的敌人，回收到对象池
		URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
		if (Pool)
		{
			double StartTime = FPlatformTime::Seconds();
			Pool->ReleaseEnemy(Enemy);
			double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
			SpawnStats.TotalDestroyCount++;
			RecordDestroyStats(ElapsedMs);

			if (bShowDebug)
			{
				UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] ReleaseSpawnedEnemy [Pool] - Released [%s] to pool, time: %.3fms"),
					*GetName(), *Enemy->GetName(), ElapsedMs);
			}
		}
	}
	else
	{
		// 普通生成的敌人，直接销毁
		DestroyEnemyDirect(Enemy);
	}
}

void ARPGEnemySpawner::LogPoolStatus()
{
	URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
	if (!Pool)
	{
		UE_LOG(LogRPGEnemySpawner, Error, TEXT("[%s] LogPoolStatus - EnemyPoolSubsystem not available"), *GetName());
		return;
	}

	const int32 Avail = Pool->GetAvailableCount(EnemyClass);

	UE_LOG(LogRPGEnemySpawner, Log, TEXT("========== [%s] Pool Status =========="), *GetName());
	UE_LOG(LogRPGEnemySpawner, Log, TEXT("  Enemy Class:    %s"), EnemyClass ? *EnemyClass->GetName() : TEXT("NULL"));
	UE_LOG(LogRPGEnemySpawner, Log, TEXT("  Available:      %d"), Avail);
	UE_LOG(LogRPGEnemySpawner, Log, TEXT("  Spawned Active: %d"), PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num());
	UE_LOG(LogRPGEnemySpawner, Log, TEXT("========================================"));

	if (GEngine && bShowDebug)
	{
		const FString Msg = FString::Printf(TEXT("[%s] Pool: %d avail / %d spawned"),
			*GetName(), Avail, PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num());
		UE_LOG(LogRPGEnemySpawner, Log, TEXT("[Spawner] Pool status - %s"), *Msg);
		Debug::Print(Msg);
	}
}

int32 ARPGEnemySpawner::GetAvailableCount() const
{
	if (!EnemyClass)
	{
		return 0;
	}

	const URPGEnemyPoolSubsystem* Pool = GetEnemyPoolSubsystem();
	return Pool ? Pool->GetAvailableCount(EnemyClass) : 0;
}

int32 ARPGEnemySpawner::GetSpawnedCount() const
{
	return PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num();
}

// ========== 性能统计方法 ==========

void ARPGEnemySpawner::RecordSpawnStats(double TimeMs, int64 MemoryBytes)
{
	SpawnStats.TotalSpawnTimeMs += TimeMs;
	SpawnStats.AverageSpawnTimeMs = SpawnStats.TotalSpawnTimeMs / SpawnStats.TotalSpawnCount;
	SpawnStats.PeakMemoryBytes = FMath::Max(SpawnStats.PeakMemoryBytes, MemoryBytes);
	SpawnStats.GCCollectionCount = GetGCCollectionCount();
}

void ARPGEnemySpawner::RecordDestroyStats(double TimeMs)
{
	SpawnStats.TotalDestroyTimeMs += TimeMs;
	SpawnStats.AverageDestroyTimeMs = SpawnStats.TotalDestroyTimeMs / SpawnStats.TotalDestroyCount;
}

void ARPGEnemySpawner::UpdateMemoryStats()
{
	// 获取当前进程内存占用
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	SpawnStats.CurrentMemoryBytes = MemStats.UsedPhysical;
}

int32 ARPGEnemySpawner::GetGCCollectionCount() const
{
	// UE 没有直接暴露 GC 计数，这里返回 0
	// 可以通过控制台命令 "stat gc" 手动查询
	return 0;
}

void ARPGEnemySpawner::ResetStats()
{
	SpawnStats.Reset();
	UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] Stats reset"), *GetName());
}

void ARPGEnemySpawner::LogStats() const
{
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("========== [%s] Performance Stats =========="), *GetName());
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Spawn Mode:          %s"), 
		SpawnMode == EEnemySpawnMode::Pool ? TEXT("Object Pool") : TEXT("Direct Spawn"));
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Total Spawns:        %d"), SpawnStats.TotalSpawnCount);
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Total Destroys:      %d"), SpawnStats.TotalDestroyCount);
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Avg Spawn Time:      %.3f ms"), SpawnStats.AverageSpawnTimeMs);
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Avg Destroy Time:    %.3f ms"), SpawnStats.AverageDestroyTimeMs);
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Peak Memory:         %.2f MB"), SpawnStats.PeakMemoryBytes / (1024.0 * 1024.0));
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Current Memory:      %.2f MB"), SpawnStats.CurrentMemoryBytes / (1024.0 * 1024.0));
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  GC Collections:      %d"), SpawnStats.GCCollectionCount);
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("  Active Enemies:      %d"), PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num());
	UE_LOG(LogRPGEnemySpawner, Warning, TEXT("====================================================="));
}

// ========== 普通生成方法 ==========

ARPGEnemyCharacter* ARPGEnemySpawner::SpawnEnemyDirect()
{
	if (!EnemyClass)
	{
		UE_LOG(LogRPGEnemySpawner, Warning, TEXT("[%s] SpawnEnemyDirect - EnemyClass is null"), *GetName());
		return nullptr;
	}

	// 记录开始时间和内存
	double StartTime = FPlatformTime::Seconds();
	UpdateMemoryStats();

	// 直接 SpawnActor
	const FTransform SpawnTransform = CalculateSpawnTransform();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ARPGEnemyCharacter* Enemy = World->SpawnActor<ARPGEnemyCharacter>(EnemyClass, SpawnTransform, SpawnParams);

	// 记录耗时和内存
	double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	UpdateMemoryStats();

	if (Enemy)
	{
		// 标记为非对象池生成
		Enemy->Tags.AddUnique(FName("DirectSpawn"));
		DirectSpawnedEnemies.Add(Enemy);
		SpawnStats.TotalSpawnCount++;
		RecordSpawnStats(ElapsedMs, SpawnStats.CurrentMemoryBytes);

		if (bShowDebug)
		{
			UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] SpawnEnemyDirect - Spawned [%s] at %s, time: %.3fms, spawned total: %d"),
				*GetName(), *Enemy->GetName(), *SpawnTransform.GetLocation().ToString(), ElapsedMs, PoolSpawnedEnemies.Num() + DirectSpawnedEnemies.Num());
		}
	}

	return Enemy;
}

void ARPGEnemySpawner::DestroyEnemyDirect(ARPGEnemyCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}

	double StartTime = FPlatformTime::Seconds();

	// 从追踪列表中移除
	DirectSpawnedEnemies.Remove(Enemy);

	// 直接销毁
	Enemy->Destroy();

	double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	SpawnStats.TotalDestroyCount++;
	RecordDestroyStats(ElapsedMs);

	if (bShowDebug)
	{
		UE_LOG(LogRPGEnemySpawner, Log, TEXT("[%s] DestroyEnemyDirect - Destroyed [%s], time: %.3fms"),
			*GetName(), *Enemy->GetName(), ElapsedMs);
	}
}
