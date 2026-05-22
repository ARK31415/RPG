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
	URPGMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeSupportsCustomNavigation() const override { return false; }
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void ExitGame();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* StartGameButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ExitGameButton;

private:
	// 按钮点击事件处理
	void OnStartGameClicked();
	void OnExitGameClicked();
};
