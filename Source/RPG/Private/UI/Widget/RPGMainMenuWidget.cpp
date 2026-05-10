// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/RPGMainMenuWidget.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystem/RPGLoadingSubsystem.h"
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
	// 获取 GameInstance
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("URPGMainMenuWidget::StartGame - World is null"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("URPGMainMenuWidget::StartGame - GameInstance is null"));
		return;
	}

	// 获取加载子系统
	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("URPGMainMenuWidget::StartGame - LoadingSubsystem is null"));
		return;
	}

	// 异步加载主关卡（含 Slate 加载画面）
	TSoftObjectPtr<UWorld> MainLevel(FSoftObjectPath(TEXT("/Game/Maps/MainLevel")));
	LoadingSubsystem->AsyncLoadLevel(MainLevel, true, TEXT("?listen"));

	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("URPGMainMenuWidget::StartGame - Async loading MainLevel via LoadingSubsystem"));
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
