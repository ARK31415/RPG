// RPGLoadingScreenWidget - CommonUI 加载画面控件实现

#include "UI/Widget/RPGLoadingScreenWidget.h"
#include "Subsystem/RPGLoadingSubsystem.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGLoadingScreenWidget, All, All)

URPGLoadingScreenWidget::URPGLoadingScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LoadingProgressBar = nullptr;
	StageTextBlock = nullptr;
	MapNameTextBlock = nullptr;
	BackgroundImage = nullptr;
	TipTextBlock = nullptr;
}

void URPGLoadingScreenWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 订阅 LoadingSubsystem 的进度委托
	if (URPGLoadingSubsystem* LS = GetLoadingSubsystem())
	{
		LS->OnLoadingProgressChanged.AddDynamic(this, &URPGLoadingScreenWidget::OnLoadingProgressChanged);
		LS->OnLoadingStateChanged.AddDynamic(this, &URPGLoadingScreenWidget::OnLoadingStateChanged);

		// 立即同步当前进度
		OnLoadingProgressChanged(LS->GetCurrentProgress());

		UE_LOG(LogRPGLoadingScreenWidget, Log, TEXT("NativeOnActivated - Subscribed to LoadingSubsystem delegates"));
	}

	// 启动提示文本轮播
	if (LoadingTips.Num() > 0 && TipRotationInterval > 0.0f)
	{
		CurrentTipIndex = 0;
		RotateTipText();

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				TipRotationTimer,
				this,
				&URPGLoadingScreenWidget::RotateTipText,
				TipRotationInterval,
				true
			);
		}
	}
}

void URPGLoadingScreenWidget::NativeOnDeactivated()
{
	// 取消订阅委托
	if (URPGLoadingSubsystem* LS = GetLoadingSubsystem())
	{
		LS->OnLoadingProgressChanged.RemoveDynamic(this, &URPGLoadingScreenWidget::OnLoadingProgressChanged);
		LS->OnLoadingStateChanged.RemoveDynamic(this, &URPGLoadingScreenWidget::OnLoadingStateChanged);
	}

	// 清理提示轮播 Timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TipRotationTimer);
	}

	Super::NativeOnDeactivated();

	UE_LOG(LogRPGLoadingScreenWidget, Log, TEXT("NativeOnDeactivated - Unsubscribed from LoadingSubsystem delegates"));
}

// ================================================================
//  委托回调
// ================================================================

void URPGLoadingScreenWidget::OnLoadingProgressChanged(const FRPGLoadingProgress& Progress)
{
	// 更新进度条
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(Progress.Progress);
	}

	// 更新阶段文本
	if (StageTextBlock)
	{
		StageTextBlock->SetText(Progress.CurrentStageText);
	}
}

void URPGLoadingScreenWidget::OnLoadingStateChanged(ERPGLoadingState NewState)
{
	UE_LOG(LogRPGLoadingScreenWidget, Log, TEXT("OnLoadingStateChanged - New state: %d"), static_cast<uint8>(NewState));

	// 加载完成时，可以触发淡出动画或由 UIManager 弹出此 Widget
	if (NewState == ERPGLoadingState::Complete || NewState == ERPGLoadingState::Idle)
	{
		// 可选：触发淡出动画（蓝图子类可覆写此逻辑）
	}
}

// ================================================================
//  内部辅助
// ================================================================

void URPGLoadingScreenWidget::RotateTipText()
{
	if (LoadingTips.Num() == 0 || !TipTextBlock)
	{
		return;
	}

	TipTextBlock->SetText(LoadingTips[CurrentTipIndex]);
	CurrentTipIndex = (CurrentTipIndex + 1) % LoadingTips.Num();
}

URPGLoadingSubsystem* URPGLoadingScreenWidget::GetLoadingSubsystem() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<URPGLoadingSubsystem>();
}
