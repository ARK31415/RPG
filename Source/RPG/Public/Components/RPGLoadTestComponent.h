// RPGLoadTestComponent - 关卡加载性能测试组件
// 用于对比异步和同步加载的性能差异

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPGLoadTestComponent.generated.h"

/**
 * 关卡加载性能测试组件
 * 用于对比异步和同步加载的性能差异
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API URPGLoadTestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URPGLoadTestComponent();

	/** 测试地图列表（软引用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Test")
	TArray<TSoftObjectPtr<UWorld>> TestMaps;

	/** 每个地图测试次数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Test")
	int32 TestsPerMap = 3;

	/** 执行同步加载测试 */
	UFUNCTION(BlueprintCallable, Category = "Load Test")
	void RunSyncLoadTest();

	/** 执行异步加载测试（需手动触发多次） */
	UFUNCTION(BlueprintCallable, Category = "Load Test")
	void RunAsyncLoadTest();

	/** 打印测试报告 */
	UFUNCTION(BlueprintCallable, Category = "Load Test")
	void PrintTestReport() const;
};
