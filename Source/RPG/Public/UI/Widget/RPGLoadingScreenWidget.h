// RPGLoadingScreenWidget - CommonUI 加载画面控件
// 用于关卡流式加载时显示进度，推入 Modal 层栈

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGWidget_ActivatableBase.h"
#include "RPGLoadingScreenWidget.generated.h"

class URPGLoadingSubsystem;
class UProgressBar;
class UTextBlock;
class UImage;

/**
 * 加载画面 Widget
 *
 * 监听 URPGLoadingSubsystem 的进度/状态委托，实时更新进度条和提示文本。
 * 由 RPGUIManagerSubsystem 推入 Modal 层栈（仅在流式加载期间显示）。
 */
UCLASS()
class RPG_API URPGLoadingScreenWidget : public URPGWidget_ActivatableBase
{
	GENERATED_BODY()

public:
	URPGLoadingScreenWidget(const FObjectInitializer& ObjectInitializer);

	// ---- UCommonActivatableWidget Interface ----
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	// ================================================================
	//  UMG 绑定控件（由蓝图设计器绑定）
	// ================================================================

	/** 加载进度条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	/** 阶段提示文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StageTextBlock;

	/** 地图/关卡名称文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> MapNameTextBlock;

	/** 加载画面背景图 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	/** 提示轮播文本（可选） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TipTextBlock;

	// ================================================================
	//  配置属性（可由蓝图子类覆盖）
	// ================================================================

	/** 提示文本轮播间隔（秒），0 表示不轮播 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LoadingScreen")
	float TipRotationInterval = 5.0f;

	/** 加载提示文本列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LoadingScreen")
	TArray<FText> LoadingTips;

private:
	// ================================================================
	//  委托回调
	// ================================================================

	/** 加载进度更新回调 */
	UFUNCTION()
	void OnLoadingProgressChanged(const FRPGLoadingProgress& Progress);

	/** 加载状态变更回调 */
	UFUNCTION()
	void OnLoadingStateChanged(ERPGLoadingState NewState);

	// ================================================================
	//  内部辅助
	// ================================================================

	/** 轮换提示文本 */
	void RotateTipText();

	/** 获取 LoadingSubsystem */
	URPGLoadingSubsystem* GetLoadingSubsystem() const;

	// ================================================================
	//  内部数据
	// ================================================================

	/** 当前提示索引 */
	int32 CurrentTipIndex = 0;

	/** 提示轮播 Timer 句柄 */
	FTimerHandle TipRotationTimer;
};
