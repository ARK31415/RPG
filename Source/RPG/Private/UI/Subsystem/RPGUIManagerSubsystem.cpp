// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.h"
#include "CommonActivatableWidget.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/RPGHUDWidget.h"
#include "UI/Widget/RPGMainMenuWidget.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGUIManagerSubsystem, All, All)

UPrimaryGameLayout* URPGUIManagerSubsystem::GetPrimaryGameLayout(APlayerController* Controller) const
{
	if (!Controller)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("GetPrimaryGameLayout - Controller is null"));
		return nullptr;
	}

	UPrimaryGameLayout* Layout = Cast<UPrimaryGameLayout>(Controller->GetHUD());
	if (!Layout)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("GetPrimaryGameLayout - PrimaryGameLayout not found in HUD"));
		return nullptr;
	}

	return Layout;
}

void URPGUIManagerSubsystem::ShowMainMenu(APlayerController* Controller)
{
	if (!Controller)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowMainMenu - Controller is null"));
		return;
	}

	UPrimaryGameLayout* Layout = GetPrimaryGameLayout(Controller);
	if (!Layout)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowMainMenu - Failed to get PrimaryGameLayout"));
		return;
	}

	UCommonActivatableWidgetContainerBase* MenuLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_Menu);
	if (!MenuLayer)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowMainMenu - Menu Layer is null"));
		return;
	}

	MenuLayer->AddWidget<URPGMainMenuWidget>(URPGMainMenuWidget::StaticClass());
	UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("ShowMainMenu - Main menu displayed"));
}

void URPGUIManagerSubsystem::ShowHUD(APlayerController* Controller)
{
	if (!Controller)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowHUD - Controller is null"));
		return;
	}

	UPrimaryGameLayout* Layout = GetPrimaryGameLayout(Controller);
	if (!Layout)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowHUD - Failed to get PrimaryGameLayout"));
		return;
	}

	UCommonActivatableWidgetContainerBase* GameLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_Game);
	if (!GameLayer)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("ShowHUD - Game Layer is null"));
		return;
	}

	GameLayer->AddWidget<URPGHUDWidget>(URPGHUDWidget::StaticClass());
	UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("ShowHUD - HUD displayed"));
}

void URPGUIManagerSubsystem::HideAllUI(APlayerController* Controller)
{
	if (!Controller)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("HideAllUI - Controller is null"));
		return;
	}

	UPrimaryGameLayout* Layout = GetPrimaryGameLayout(Controller);
	if (!Layout)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("HideAllUI - Failed to get PrimaryGameLayout"));
		return;
	}

	// 清空所有Layer
	if (UCommonActivatableWidgetContainerBase* GameLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_Game))
	{
		GameLayer->ClearWidgets();
	}
	if (UCommonActivatableWidgetContainerBase* GameMenuLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_GameMenu))
	{
		GameMenuLayer->ClearWidgets();
	}
	if (UCommonActivatableWidgetContainerBase* MenuLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_Menu))
	{
		MenuLayer->ClearWidgets();
	}
	if (UCommonActivatableWidgetContainerBase* ModalLayer = Layout->GetLayerWidget(RPGGameplayTags::UI_Layer_Modal))
	{
		ModalLayer->ClearWidgets();
	}

	UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("HideAllUI - All UI layers cleared"));
}
