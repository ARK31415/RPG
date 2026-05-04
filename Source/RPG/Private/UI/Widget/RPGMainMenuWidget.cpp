// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/RPGMainMenuWidget.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGMainMenuWidget, All, All)

URPGMainMenuWidget::URPGMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 设置默认值
	TitleText = nullptr;
	StartGameButton = nullptr;
	ExitGameButton = nullptr;
}

bool URPGMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// 绑定按钮点击事件
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnStartGameClicked);
	}

	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnExitGameClicked);
	}

	// 设置标题文本
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("RPG Game")));
	}

	return true;
}

void URPGMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 确保按钮可用
	if (StartGameButton)
	{
		StartGameButton->SetIsEnabled(true);
	}

	if (ExitGameButton)
	{
		ExitGameButton->SetIsEnabled(true);
	}
}

void URPGMainMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 清理按钮事件绑定
	if (StartGameButton)
	{
		StartGameButton->OnClicked.Clear();
	}

	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.Clear();
	}
}

void URPGMainMenuWidget::StartGame()
{
	// 获取当前世界
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("URPGMainMenuWidget::StartGame - World is null"));
		return;
	}

	// 获取玩家控制器
	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (!PC)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("URPGMainMenuWidget::StartGame - PlayerController is null"));
		return;
	}

	// 加载主关卡
	UGameplayStatics::OpenLevel(World, TEXT("MainLevel"), true, TEXT("?listen"));
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("URPGMainMenuWidget::StartGame - Loading MainLevel"));
}

void URPGMainMenuWidget::ExitGame()
{
	// 退出游戏
	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(PC, nullptr, EQuitPreference::Quit, false);
	}
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("URPGMainMenuWidget::ExitGame - Quitting game"));
}

void URPGMainMenuWidget::OnStartGameClicked()
{
	StartGame();
}

void URPGMainMenuWidget::OnExitGameClicked()
{
	ExitGame();
}
