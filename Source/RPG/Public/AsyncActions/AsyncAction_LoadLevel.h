// AsyncAction_LoadLevel - 蓝图异步加载关卡节点
// 遵循 UAsyncAction_PushSoftWidget 的现有异步模式

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Subsystem/RPGLoadingSubsystem.h"
#include "AsyncAction_LoadLevel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadLevelProgressDelegate, const FRPGLoadingProgress&, Progress);

/**
 * 异步加载关卡的蓝图节点
 *
 * 蓝图用法：
 *   1. 调用 AsyncLoadLevel 创建节点
 *   2. 绑定 OnLoadStarted / OnProgressUpdated / OnLoadCompleted 事件
 *   3. 节点自动激活并开始加载
 */
UCLASS()
class RPG_API UAsyncAction_LoadLevel : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * 异步加载目标关卡
	 * @param WorldContextObject 世界上下文
	 * @param TargetLevel 目标关卡软引用
	 * @param Options URL 参数字符串（如 "?listen"）
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject",
		BlueprintInternalUseOnly = true, DisplayName = "Async Load Level"))
	static UAsyncAction_LoadLevel* AsyncLoadLevel(const UObject* WorldContextObject,
	                                               TSoftObjectPtr<UWorld> TargetLevel,
	                                               FString Options = TEXT(""));

	virtual void Activate() override;

	/** 加载开始时触发 */
	UPROPERTY(BlueprintAssignable)
	FOnLoadLevelProgressDelegate OnLoadStarted;

	/** 加载进度更新时触发 */
	UPROPERTY(BlueprintAssignable)
	FOnLoadLevelProgressDelegate OnProgressUpdated;

	/** 加载完成时触发 */
	UPROPERTY(BlueprintAssignable)
	FOnLoadLevelProgressDelegate OnLoadCompleted;

private:
	/** 进度更新回调 */
	UFUNCTION()
	void HandleProgressChanged(const FRPGLoadingProgress& Progress);

	/** 状态变更回调 */
	UFUNCTION()
	void HandleStateChanged(ERPGLoadingState NewState);

	/** 缓存的世界上下文 */
	TWeakObjectPtr<UWorld> CachedWorld;

	/** 缓存的目标关卡 */
	TSoftObjectPtr<UWorld> CachedTargetLevel;

	/** 缓存的 Options */
	FString CachedOptions;

	/** 是否已触发加载开始 */
	bool bHasStarted = false;
};
