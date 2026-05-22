// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/RPGBaseController.h"
#include "RPGMenuPlayerController.generated.h"

class UWidgetLayout_Base;
class URPGMainMenuWidget;

/**
 * 主菜单 PlayerController（轻量级）
 * 职责：创建 PrimaryGameLayout → 注册到 UIManager → 将 MainMenuWidget 推入 Frontend 层栈
 */
UCLASS()
class RPG_API ARPGMenuPlayerController : public ARPGBaseController
{
	GENERATED_BODY()

public:
	ARPGMenuPlayerController();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|MainMenu")
	TSubclassOf<UWidgetLayout_Base> PrimaryGameLayoutClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|MainMenu")
	TSoftClassPtr<URPGMainMenuWidget> MainMenuWidgetClass;

private:
	void ShowMainMenu();
};
