// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RPGWidget_ActivatableBase.h"
#include "RPGMainMenuWidget.generated.h"
/**
 * 主菜单Widget，显示游戏标题和主要选项
 */
UCLASS()
class RPG_API URPGMainMenuWidget : public URPGWidget_ActivatableBase
{
	GENERATED_BODY()

public:
	// 构造函数
	URPGMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	// 初始化Widget
	virtual bool Initialize() override;

	// 激活Widget
	virtual void NativeOnActivated() override;

	// 停用Widget
	virtual void NativeOnDeactivated() override;

	// 开始游戏
	UFUNCTION(BlueprintCallable)
	void StartGame();

	// 退出游戏
	UFUNCTION(BlueprintCallable)
	void ExitGame();

protected:
	// 游戏标题文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TitleText;

	// 开始游戏按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* StartGameButton;

	// 退出游戏按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ExitGameButton;

private:
	// 按钮点击事件处理
	void OnStartGameClicked();
	void OnExitGameClicked();
};
