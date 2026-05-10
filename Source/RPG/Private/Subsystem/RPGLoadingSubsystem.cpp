// RPGLoadingSubsystem - 场景异步加载子系统实现

#include "Subsystem/RPGLoadingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/SViewport.h"
#include "Engine/GameViewportClient.h"
#include "Rendering/DrawElements.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGLoadingSubsystem, All, All)

// ================================================================
//  Slate 加载画面 Widget
// ================================================================

/**
 * 纯 Slate 全屏加载画面，存活于 Slate 渲染线程，跨越 World 切换
 */
class SRPGLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRPGLoadingScreen) {}
		SLATE_ARGUMENT(FString, MapName)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		MapName = InArgs._MapName;

		ChildSlot
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 1.0f))
			.Padding(0)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)

				// 背景层 — 纯色
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 1.0f))
				]

				// 居中内容
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					// 标题文本
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 30)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("LOADING...")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
					]

					// 地图名称
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 40)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(MapName))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.7f, 1.0f))
					]

					// 旋转指示器 (Throbber)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SCircularThrobber)
						.Radius(32.0f)
					]
				]
			]
		];
	}

private:
	FString MapName;
};

// ================================================================
//  URPGLoadingSubsystem
// ================================================================

void URPGLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 绑定引擎关卡加载委托（GameInstance 生命周期，跨越世界切换）
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &URPGLoadingSubsystem::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &URPGLoadingSubsystem::OnPostLoadMapWithWorld);

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("URPGLoadingSubsystem::Initialize - Subsystem initialized"));
}

void URPGLoadingSubsystem::Deinitialize()
{
	// 解绑引擎委托
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	// 确保 Slate 加载画面被清理
	HideSlateLoadingScreen();

	// 清理流式子关卡轮询 Timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SubLevelPollTimer);
	}

	ActiveSubLevels.Empty();

	Super::Deinitialize();

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("URPGLoadingSubsystem::Deinitialize - Subsystem deinitialized"));
}

bool URPGLoadingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 仅在非 Dedicated Server 上创建
	if (const UGameInstance* GI = Cast<UGameInstance>(Outer))
	{
		return !GI->IsDedicatedServerInstance();
	}
	return false;
}

// ================================================================
//  关卡过渡（OpenLevel）
// ================================================================

void URPGLoadingSubsystem::AsyncLoadLevel(TSoftObjectPtr<UWorld> TargetLevel, bool bAbsolute, FString Options)
{
	if (TargetLevel.IsNull())
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("AsyncLoadLevel - TargetLevel is null"));
		return;
	}

	if (CurrentState == ERPGLoadingState::TransitioningLevel)
	{
		UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("AsyncLoadLevel - Already transitioning, ignoring duplicate request"));
		return;
	}

	// 解析关卡路径
	const FString LevelPath = TargetLevel.ToSoftObjectPath().ToString();
	PendingLevelPath = LevelPath;
	PendingLevelOptions = Options;

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("AsyncLoadLevel - Starting async load for: %s"), *LevelPath);

	// 设置状态并广播
	SetLoadingState(ERPGLoadingState::TransitioningLevel);
	UpdateProgress(0.05f, FText::FromString(TEXT("Preparing...")));

	// 可选：预加载关卡资产（通过 Asset Manager）
	UAssetManager& AssetManager = UAssetManager::Get();
	FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();

	StreamableManager.RequestAsyncLoad(
		TargetLevel.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, LevelPath]()
		{
			UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("AsyncLoadLevel - Assets preloaded for: %s, opening level"), *LevelPath);

			UpdateProgress(0.20f, FText::FromString(TEXT("Opening Level...")));

			// 调用 OpenLevel —— 这会触发 PreLoadMap → 新的 World 加载 → PostLoadMap
			if (UWorld* World = GetWorld())
			{
				UGameplayStatics::OpenLevel(World, FName(*PendingLevelPath), true, PendingLevelOptions);
			}
			else
			{
				UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("AsyncLoadLevel - World is null, cannot open level"));
				SetLoadingState(ERPGLoadingState::Idle);
			}
		}),
		FStreamableManager::AsyncLoadHighPriority
	);
}

float URPGLoadingSubsystem::SyncLoadLevel(TSoftObjectPtr<UWorld> TargetLevel, bool bAbsolute, FString Options)
{
	if (TargetLevel.IsNull())
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("SyncLoadLevel - TargetLevel is null"));
		return 0.0f;
	}

	if (CurrentState != ERPGLoadingState::Idle)
	{
		UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("SyncLoadLevel - System not idle, current state: %d"),
			static_cast<uint8>(CurrentState));
		return 0.0f;
	}

	const FString LevelPath = TargetLevel.ToSoftObjectPath().ToString();
	const double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("SyncLoadLevel - Starting SYNC load for: %s"), *LevelPath);

	// 同步加载：直接调用 OpenLevel（阻塞当前线程）
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName(*LevelPath), bAbsolute, Options);
	}
	else
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("SyncLoadLevel - World is null"));
		return 0.0f;
	}

	// 计算耗时（OpenLevel 返回时关卡已加载完成）
	const double EndTime = FPlatformTime::Seconds();
	LastLoadDuration = static_cast<float>(EndTime - StartTime);

	// 记录性能数据
	if (!LoadPerformanceRecords.Contains(LevelPath))
	{
		LoadPerformanceRecords.Add(LevelPath, TArray<float>());
	}
	LoadPerformanceRecords[LevelPath].Add(LastLoadDuration);

	UE_LOG(LogRPGLoadingSubsystem, Warning,
		TEXT("SyncLoadLevel - COMPLETED in %.3f seconds for: %s"),
		LastLoadDuration, *LevelPath);

	return LastLoadDuration;
}

// ================================================================
//  流式加载（子关卡动态加载/卸载）
// ================================================================

void URPGLoadingSubsystem::LoadSubLevel(FName SubLevelName, TSoftObjectPtr<UWorld> SubLevel,
                                        FVector Location, FRotator Rotation)
{
	if (SubLevel.IsNull())
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("LoadSubLevel - SubLevel is null for: %s"), *SubLevelName.ToString());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("LoadSubLevel - World is null"));
		return;
	}

	if (ActiveSubLevels.Contains(SubLevelName))
	{
		UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("LoadSubLevel - SubLevel '%s' is already loaded"), *SubLevelName.ToString());
		return;
	}

	SetLoadingState(ERPGLoadingState::StreamingLevel);
	UpdateProgress(0.1f, FText::Format(FText::FromString(TEXT("Loading {0}...")), FText::FromName(SubLevelName)));

	// 使用 ULevelStreamingDynamic 异步加载子关卡
	bool bOutSuccess = false;
	ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		World,
		SubLevel,
		Location,
		Rotation,
		bOutSuccess
	);

	if (bOutSuccess && StreamingLevel)
	{
		ActiveSubLevels.Add(SubLevelName, StreamingLevel);
		UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("LoadSubLevel - Started loading sub-level: %s"), *SubLevelName.ToString());

		// 启动 Timer 轮询加载状态
		World->GetTimerManager().SetTimer(
			SubLevelPollTimer,
			this,
			&URPGLoadingSubsystem::PollSubLevelStatus,
			0.1f,  // 每 100ms 轮询一次
			true   // 循环
		);
	}
	else
	{
		UE_LOG(LogRPGLoadingSubsystem, Error, TEXT("LoadSubLevel - Failed to start loading sub-level: %s"), *SubLevelName.ToString());
		SetLoadingState(ERPGLoadingState::Idle);
	}
}

void URPGLoadingSubsystem::UnloadSubLevel(FName SubLevelName)
{
	TObjectPtr<ULevelStreamingDynamic>* Found = ActiveSubLevels.Find(SubLevelName);
	if (!Found || !*Found)
	{
		UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("UnloadSubLevel - SubLevel '%s' not found"), *SubLevelName.ToString());
		return;
	}

	ULevelStreamingDynamic* StreamingLevel = *Found;
	StreamingLevel->SetShouldBeLoaded(false);
	ActiveSubLevels.Remove(SubLevelName);

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("UnloadSubLevel - Unloading sub-level: %s"), *SubLevelName.ToString());

	// 如果没有其他活跃的子关卡，回到 Idle
	if (ActiveSubLevels.IsEmpty())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SubLevelPollTimer);
		}
		SetLoadingState(ERPGLoadingState::Idle);
	}
}

// ================================================================
//  Slate 全屏加载画面
// ================================================================

void URPGLoadingSubsystem::ShowSlateLoadingScreen(const FString& MapName)
{
	if (SlateLoadingScreenWidget.IsValid())
	{
		UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("ShowSlateLoadingScreen - Already showing"));
		return;
	}

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("ShowSlateLoadingScreen - Showing for map: %s"), *MapName);

	// 构造 Slate Widget
	SlateLoadingScreenWidget = SNew(SRPGLoadingScreen).MapName(MapName);

	// 添加到游戏视口（0 层级，覆盖所有内容）
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(
			SlateLoadingScreenWidget.ToSharedRef(),
			0  // Z-Order: 最高层级，覆盖所有
		);
	}
}

void URPGLoadingSubsystem::HideSlateLoadingScreen()
{
	if (!SlateLoadingScreenWidget.IsValid())
	{
		return;
	}

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("HideSlateLoadingScreen - Hiding"));

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			SlateLoadingScreenWidget.ToSharedRef()
		);
	}

	SlateLoadingScreenWidget.Reset();
}

// ================================================================
//  引擎委托回调
// ================================================================

void URPGLoadingSubsystem::OnPreLoadMap(const FString& MapName)
{
	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("OnPreLoadMap - Loading map: %s"), *MapName);

	// 显示 Slate 全屏加载画面（当前 World 即将销毁，UMG Widget 会随之消失）
	ShowSlateLoadingScreen(MapName);
}

void URPGLoadingSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// 计算异步加载总耗时
	double EndTime = FPlatformTime::Seconds();
	float AsyncDuration = 0.0f;
	
	if (LoadingStartTime > 0.0)
	{
		AsyncDuration = static_cast<float>(EndTime - LoadingStartTime);
		LastLoadDuration = AsyncDuration;

		// 记录性能数据
		if (!PendingLevelPath.IsEmpty())
		{
			if (!LoadPerformanceRecords.Contains(PendingLevelPath))
			{
				LoadPerformanceRecords.Add(PendingLevelPath, TArray<float>());
			}
			LoadPerformanceRecords[PendingLevelPath].Add(AsyncDuration);

			UE_LOG(LogRPGLoadingSubsystem, Warning,
				TEXT("OnPostLoadMapWithWorld - ASYNC COMPLETED in %.3f seconds for: %s"),
				AsyncDuration, *PendingLevelPath);
		}
	}

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("OnPostLoadMapWithWorld - Map loaded: %s"),
		LoadedWorld ? *LoadedWorld->GetName() : TEXT("null"));

	// 隐藏 Slate 加载画面
	HideSlateLoadingScreen();

	// 清空暂存数据
	PendingLevelPath.Empty();
	PendingLevelOptions.Empty();

	// 短暂设为 Complete，然后回到 Idle
	SetLoadingState(ERPGLoadingState::Complete);

	// 延迟切回 Idle（给 UI 过渡留一点时间）
	if (LoadedWorld)
	{
		FTimerHandle DelayTimer;
		LoadedWorld->GetTimerManager().SetTimer(
			DelayTimer,
			[this]()
			{
				if (CurrentState == ERPGLoadingState::Complete)
				{
					SetLoadingState(ERPGLoadingState::Idle);
				}
			},
			0.5f,
			false
		);
	}

	UpdateProgress(1.0f, FText::FromString(TEXT("Complete")));
}

float URPGLoadingSubsystem::GetAverageLoadDuration(const FString& LevelPath) const
{
	const TArray<float>* Records = LoadPerformanceRecords.Find(LevelPath);
	if (!Records || Records->Num() == 0)
	{
		return 0.0f;
	}

	float Sum = 0.0f;
	for (float Duration : *Records)
	{
		Sum += Duration;
	}
	return Sum / Records->Num();
}

void URPGLoadingSubsystem::PrintPerformanceReport() const
{
	UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("========== Loading Performance Report =========="));
	
	for (const auto& Pair : LoadPerformanceRecords)
	{
		const FString& LevelPath = Pair.Key;
		const TArray<float>& Records = Pair.Value;
		
		float Sum = 0.0f;
		float Min = TNumericLimits<float>::Max();
		float Max = TNumericLimits<float>::Lowest();
		
		for (float Duration : Records)
		{
			Sum += Duration;
			Min = FMath::Min(Min, Duration);
			Max = FMath::Max(Max, Duration);
		}
		
		const float Avg = Sum / Records.Num();
		
		UE_LOG(LogRPGLoadingSubsystem, Warning,
			TEXT("Map: %s"), *LevelPath);
		UE_LOG(LogRPGLoadingSubsystem, Warning,
			TEXT("  Tests: %d | Avg: %.3fs | Min: %.3fs | Max: %.3fs"),
			Records.Num(), Avg, Min, Max);
	}
	
	UE_LOG(LogRPGLoadingSubsystem, Warning, TEXT("================================================"));
}

// ================================================================
//  内部辅助
// ================================================================

void URPGLoadingSubsystem::SetLoadingState(ERPGLoadingState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;

	UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("SetLoadingState - State changed to: %d"), static_cast<uint8>(NewState));

	// 广播状态变更
	OnLoadingStateChanged.Broadcast(NewState);
}

void URPGLoadingSubsystem::UpdateProgress(float InProgress, const FText& StageText)
{
	CurrentProgress.Progress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	CurrentProgress.CurrentStageText = StageText;

	// 计算已耗时
	if (CurrentState == ERPGLoadingState::Idle)
	{
		LoadingStartTime = 0.0;
		CurrentProgress.ElapsedTime = 0.0f;
	}
	else
	{
		if (LoadingStartTime == 0.0)
		{
			LoadingStartTime = FPlatformTime::Seconds();
		}
		CurrentProgress.ElapsedTime = static_cast<float>(FPlatformTime::Seconds() - LoadingStartTime);
	}

	// 广播进度更新
	OnLoadingProgressChanged.Broadcast(CurrentProgress);
}

void URPGLoadingSubsystem::PollSubLevelStatus()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 检查所有活跃子关卡的加载状态
	int32 LoadedCount = 0;
	const int32 TotalCount = ActiveSubLevels.Num();

	for (auto& Pair : ActiveSubLevels)
	{
		ULevelStreamingDynamic* StreamingLevel = Pair.Value;
		if (StreamingLevel && StreamingLevel->IsLevelLoaded())
		{
			++LoadedCount;
		}
	}

	// 计算总进度（子关卡加载占 10%~100%）
	const float SubProgress = (TotalCount > 0) ? (static_cast<float>(LoadedCount) / static_cast<float>(TotalCount)) : 1.0f;
	const float FinalProgress = 0.1f + SubProgress * 0.9f;

	FText StageText = FText::Format(
		FText::FromString(TEXT("Loading sub-levels ({0}/{1})...")),
		FText::AsNumber(LoadedCount),
		FText::AsNumber(TotalCount)
	);

	UpdateProgress(FinalProgress, StageText);

	// 全部加载完成
	if (LoadedCount >= TotalCount && TotalCount > 0)
	{
		SetLoadingState(ERPGLoadingState::Complete);
		UpdateProgress(1.0f, FText::FromString(TEXT("Complete")));

		World->GetTimerManager().ClearTimer(SubLevelPollTimer);

		// 延迟切回 Idle
		FTimerHandle DelayTimer;
		World->GetTimerManager().SetTimer(
			DelayTimer,
			[this]()
			{
				if (CurrentState == ERPGLoadingState::Complete)
				{
					SetLoadingState(ERPGLoadingState::Idle);
				}
			},
			0.5f,
			false
		);

		UE_LOG(LogRPGLoadingSubsystem, Log, TEXT("PollSubLevelStatus - All sub-levels loaded"));
	}
}
