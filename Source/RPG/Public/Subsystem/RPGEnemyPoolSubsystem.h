// RPGEnemyPoolSubsystem - 敌人对象池子系统
// 复用已死亡的敌人 Actor，避免频繁 Spawn/Destroy 的性能开销

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RPGEnemyPoolSubsystem.generated.h"

class ARPGEnemyCharacter;

/**
 * 敌人对象池子系统（UWorldSubsystem）
 * 
 * 用法：
 *   // 从池中获取或生成新敌人
 *   ARPGEnemyCharacter* Enemy = GetWorld()->GetSubsystem<URPGEnemyPoolSubsystem>()
 *       ->AcquireEnemy(EnemyClass, SpawnTransform);
 *
 *   // 死亡后归还到池中（替代 Destroy）
 *   GetWorld()->GetSubsystem<URPGEnemyPoolSubsystem>()->ReleaseEnemy(Enemy);
 */
UCLASS()
class RPG_API URPGEnemyPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	/**
	 * 从对象池获取一个敌人。如果池中无可用实例则新建。
	 * @param EnemyClass 敌人蓝图类
	 * @param SpawnTransform 生成位置/旋转
	 * @return 可用的敌人 Actor（已激活）
	 */
	UFUNCTION(BlueprintCallable, Category="RPG|Pool")
	ARPGEnemyCharacter* AcquireEnemy(TSubclassOf<ARPGEnemyCharacter> EnemyClass, const FTransform& SpawnTransform);

	/**
	 * 将敌人归还到对象池（替代 Destroy）。
	 * 敌人会被隐藏、禁用碰撞和 Tick。
	 */
	UFUNCTION(BlueprintCallable, Category="RPG|Pool")
	void ReleaseEnemy(ARPGEnemyCharacter* Enemy);

	/** 预热池：预先生成指定数量的敌人 */
	UFUNCTION(BlueprintCallable, Category="RPG|Pool")
	void WarmPool(TSubclassOf<ARPGEnemyCharacter> EnemyClass, int32 Count);

	/** 获取当前池中可用数量 */
	UFUNCTION(BlueprintPure, Category="RPG|Pool")
	int32 GetAvailableCount(TSubclassOf<ARPGEnemyCharacter> EnemyClass) const;

private:
	// 每个类对应一个可用实例队列
	TMap<UClass*, TArray<TObjectPtr<ARPGEnemyCharacter>>> Pool;

	// 将敌人置为非活跃状态
	void DeactivateEnemy(ARPGEnemyCharacter* Enemy);

	// 将敌人从池中激活
	void ActivateEnemy(ARPGEnemyCharacter* Enemy, const FTransform& SpawnTransform);
};
