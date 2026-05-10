// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGEnemySpawner.generated.h"

class ARPGEnemyCharacter;
class URPGEnemyPoolSubsystem;
class UArrowComponent;
class UBoxComponent;

/** 敌人生成模式 */
UENUM(BlueprintType)
enum class EEnemySpawnMode : uint8
{
	Pool    UMETA(DisplayName = "对象池模式"),
	Direct  UMETA(DisplayName = "普通生成模式")
};

/** 性能统计结构体 */
USTRUCT(BlueprintType)
struct FEnemySpawnStats
{
	GENERATED_BODY()

	// 生成统计
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalSpawnCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalDestroyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	double TotalSpawnTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	double TotalDestroyTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	double AverageSpawnTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	double AverageDestroyTimeMs = 0.0;

	// 内存统计
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int64 PeakMemoryBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int64 CurrentMemoryBytes = 0;

	// GC 统计
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 GCCollectionCount = 0;

	// 重置统计
	void Reset()
	{
		TotalSpawnCount = 0;
		TotalDestroyCount = 0;
		TotalSpawnTimeMs = 0.0;
		TotalDestroyTimeMs = 0.0;
		AverageSpawnTimeMs = 0.0;
		AverageDestroyTimeMs = 0.0;
		PeakMemoryBytes = 0;
		CurrentMemoryBytes = 0;
		GCCollectionCount = 0;
	}
};

UCLASS(BlueprintType, Blueprintable)
class RPG_API ARPGEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	ARPGEnemySpawner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	void WarmPool();

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	ARPGEnemyCharacter* SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	TArray<ARPGEnemyCharacter*> SpawnMultipleEnemies(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	void ReleaseAllSpawnedEnemies();

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	void ReleaseSpawnedEnemy(ARPGEnemyCharacter* Enemy);

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	void LogPoolStatus();

	UFUNCTION(BlueprintPure, Category = "RPG|EnemySpawner")
	int32 GetAvailableCount() const;

	UFUNCTION(BlueprintPure, Category = "RPG|EnemySpawner")
	int32 GetSpawnedCount() const;

	// 新增：普通生成接口
	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	ARPGEnemyCharacter* SpawnEnemyDirect();

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner")
	void DestroyEnemyDirect(ARPGEnemyCharacter* Enemy);

	// 新增：性能统计
	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner|Stats")
	void ResetStats();

	UFUNCTION(BlueprintCallable, Category = "RPG|EnemySpawner|Stats")
	void LogStats() const;

	UFUNCTION(BlueprintPure, Category = "RPG|EnemySpawner|Stats")
	FEnemySpawnStats GetStats() const { return SpawnStats; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool")
	TSubclassOf<ARPGEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool")
	int32 PreWarmCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool")
	bool bAutoWarmOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Spawn")
	bool bUseBoxComponentAsSpawnArea = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Spawn")
	FVector SpawnOffset = FVector(200.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Spawn")
	float SpawnSpreadRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Debug")
	bool bShowDebug = true;

	// 新增：自动化测试配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Auto Test")
	bool bEnableAutoTest = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Auto Test", meta = (EditCondition = "bEnableAutoTest"))
	int32 TestSpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Auto Test", meta = (EditCondition = "bEnableAutoTest"))
	float TestDelayAfterWarm = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Pool|Auto Test", meta = (EditCondition = "bEnableAutoTest"))
	float TestDelayBeforeRelease = 10.0f;

private:
	FTransform CalculateSpawnTransform() const;

	URPGEnemyPoolSubsystem* GetEnemyPoolSubsystem() const;

	// 新增：辅助方法
	void RecordSpawnStats(double TimeMs, int64 MemoryBytes);
	void RecordDestroyStats(double TimeMs);
	void UpdateMemoryStats();
	int32 GetGCCollectionCount() const;

	// 修改：使用两个数组分别追踪对象池生成和普通生成的敌人
	UPROPERTY(Transient)
	TArray<TObjectPtr<ARPGEnemyCharacter>> PoolSpawnedEnemies;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ARPGEnemyCharacter>> DirectSpawnedEnemies;

	UPROPERTY()
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY()
	TObjectPtr<UBoxComponent> SpawnAreaBox;

	// 新增：成员变量
	UPROPERTY(EditAnywhere, Category = "Enemy Pool|Mode")
	EEnemySpawnMode SpawnMode = EEnemySpawnMode::Pool;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Enemy Pool|Stats")
	FEnemySpawnStats SpawnStats;
};
