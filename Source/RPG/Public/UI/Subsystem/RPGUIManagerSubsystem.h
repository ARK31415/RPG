// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "Containers/Ticker.h"
#include "RPGUIManagerSubsystem.generated.h"

class APlayerController;
class UPrimaryGameLayout;

/**
 * RPG 项目的 UI 管理器 Subsystem。
 * 负责管理 CommonUI 四层栈（Modal → Menu → GameMenu → Game）的 Widget 生命周期。
 * 生命周期跟随 GameInstance，全局唯一。
 */
UCLASS()
class RPG_API URPGUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	/** 获取当前 LocalPlayer 对应的 PrimaryGameLayout */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UPrimaryGameLayout* GetPrimaryGameLayout(APlayerController* Controller) const;

	/** 在 Menu 层显示主菜单 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu(APlayerController* Controller);

	/** 在 Game 层显示 HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUD(APlayerController* Controller);

	/** 清空四层所有 UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllUI(APlayerController* Controller);
};
