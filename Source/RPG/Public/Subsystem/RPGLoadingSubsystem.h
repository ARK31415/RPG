// RPGLoadingSubsystem - 场景异步加载子系统
// 统一管理关卡过渡与流式加载，通过 GameInstance 生命周期跨关卡存活

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "RPGLoadingSubsystem.generated.h"

/**
 * 加载状态枚举
 */
UENUM(BlueprintType)
enum class ERPGLoadingState : uint8
{
	/** 空闲，无加载任务 */
	Idle,
	/** 正在执行关卡过渡（OpenLevel） */
	TransitioningLevel,
	/** 正在执行流式子关卡加载/卸载 */
	StreamingLevel,
	/** 加载完成（短暂停留后自动切回 Idle） */
	Complete
};

/**
 * 加载进度数据结构
 */
USTRUCT(BlueprintType)
struct RPG_API FRPGLoadingProgress
{
	GENERATED_BODY()

	/** 加载进度 0.0 ~ 1.0 */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	float Progress = 0.0f;

	/** 当前阶段描述文本 */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	FText CurrentStageText;

	/** 已耗时（秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	float ElapsedTime = 0.0f;
};

// 动态多播委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingProgressChanged, const FRPGLoadingProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingStateChanged, ERPGLoadingState, NewState);

/**
 * 场景异步加载子系统（UGameInstanceSubsystem）
 *
 * 用法：
 *   // 关卡过渡
 *   UGameInstance* GI = GetGameInstance();
 *   URPGLoadingSubsystem* LS = GI->GetSubsystem<URPGLoadingSubsystem>();
 *   LS->AsyncLoadLevel(TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/MainLevel"))));
 *
 *   // 流式子关卡
 *   LS->LoadSubLevel(TEXT("Dungeon_01"), SubLevelSoftPtr);
 *   LS->UnloadSubLevel(TEXT("Dungeon_01"));
 */
UCLASS()
class RPG_API URPGLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- USubsystem Interface ----
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ================================================================
	//  关卡过渡（OpenLevel）
	// ================================================================

	/**
	 * 异步加载目标关卡（含加载画面）
	 * @param TargetLevel 目标关卡软引用
	 * @param bAbsolute 是否使用绝对路径（默认 true，配合 Options 字符串使用）
	 * @param Options URL 参数字符串（如 "?listen"）
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading")
	void AsyncLoadLevel(TSoftObjectPtr<UWorld> TargetLevel, bool bAbsolute = true, FString Options = TEXT(""));

	/**
	 * 同步加载目标关卡（阻塞式，用于性能对比测试）
	 * @param TargetLevel 目标关卡软引用
	 * @param bAbsolute 是否使用绝对路径（默认 true）
	 * @param Options URL 参数字符串（如 "?listen"）
	 * @return 加载耗时（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading")
	float SyncLoadLevel(TSoftObjectPtr<UWorld> TargetLevel, bool bAbsolute = true, FString Options = TEXT(""));

	// ================================================================
	//  流式加载（子关卡动态加载/卸载）
	// ================================================================

	/**
	 * 异步加载子关卡
	 * @param SubLevelName 子关卡标识名称
	 * @param SubLevel 子关卡软引用
	 * @param Location 加载位置
	 * @param Rotation 加载旋转
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading")
	void LoadSubLevel(FName SubLevelName, TSoftObjectPtr<UWorld> SubLevel,
	                  FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator);

	/**
	 * 卸载子关卡
	 * @param SubLevelName 要卸载的子关卡标识名称
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading")
	void UnloadSubLevel(FName SubLevelName);

	// ================================================================
	//  状态查询
	// ================================================================

	/** 获取当前加载进度 */
	UFUNCTION(BlueprintPure, Category = "RPG|Loading")
	FRPGLoadingProgress GetCurrentProgress() const { return CurrentProgress; }

	/** 获取当前加载状态 */
	UFUNCTION(BlueprintPure, Category = "RPG|Loading")
	ERPGLoadingState GetCurrentState() const { return CurrentState; }

	/** 是否正在加载 */
	UFUNCTION(BlueprintPure, Category = "RPG|Loading")
	bool IsLoading() const { return CurrentState != ERPGLoadingState::Idle && CurrentState != ERPGLoadingState::Complete; }

	// ================================================================
	//  性能统计
	// ================================================================

	/** 获取最近一次加载耗时 */
	UFUNCTION(BlueprintPure, Category = "RPG|Loading|Performance")
	float GetLastLoadDuration() const { return LastLoadDuration; }

	/** 获取指定地图的平均加载耗时 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading|Performance")
	float GetAverageLoadDuration(const FString& LevelPath) const;

	/** 打印所有性能记录到日志 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Loading|Performance")
	void PrintPerformanceReport() const;

	// ================================================================
	//  委托
	// ================================================================

	/** 加载进度更新时广播 */
	UPROPERTY(BlueprintAssignable, Category = "RPG|Loading")
	FOnLoadingProgressChanged OnLoadingProgressChanged;

	/** 加载状态变更时广播 */
	UPROPERTY(BlueprintAssignable, Category = "RPG|Loading")
	FOnLoadingStateChanged OnLoadingStateChanged;

private:
	// ================================================================
	//  Slate 全屏加载画面（关卡过渡时使用，跨越世界切换）
	// ================================================================

	/** 显示 Slate 全屏加载画面 */
	void ShowSlateLoadingScreen(const FString& MapName);

	/** 隐藏 Slate 全屏加载画面 */
	void HideSlateLoadingScreen();

	// ================================================================
	//  引擎委托回调
	// ================================================================

	/** PreLoadMap 回调 —— 引擎即将加载新地图 */
	void OnPreLoadMap(const FString& MapName);

	/** PostLoadMapWithWorld 回调 —— 新地图加载完成，World 已就绪 */
	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

	// ================================================================
	//  内部辅助
	// ================================================================

	/** 设置当前状态并广播 */
	void SetLoadingState(ERPGLoadingState NewState);

	/** 更新进度并广播 */
	void UpdateProgress(float InProgress, const FText& StageText);

	/** 轮询子关卡加载状态（Timer 回调） */
	void PollSubLevelStatus();

	// ================================================================
	//  数据成员
	// ================================================================

	/** 当前加载状态 */
	ERPGLoadingState CurrentState = ERPGLoadingState::Idle;

	/** 当前加载进度 */
	FRPGLoadingProgress CurrentProgress;

	/** 加载开始时间（用于计算 ElapsedTime） */
	double LoadingStartTime = 0.0;

	/** 目标关卡（OpenLevel 暂存） */
	FString PendingLevelPath;

	/** 目标关卡 Options */
	FString PendingLevelOptions;

	/** Slate 加载画面 Widget 引用 */
	TSharedPtr<class SWidget> SlateLoadingScreenWidget;

	/** 当前活跃的流式子关卡映射（SubLevelName → ULevelStreamingDynamic*） */
	TMap<FName, TObjectPtr<ULevelStreamingDynamic>> ActiveSubLevels;

	/** 流式加载轮询 Timer 句柄 */
	FTimerHandle SubLevelPollTimer;

	/** 最近一次加载耗时（秒） */
	float LastLoadDuration = 0.0f;

	/** 加载性能记录（地图名 → 耗时数组） */
	TMap<FString, TArray<float>> LoadPerformanceRecords;
};
