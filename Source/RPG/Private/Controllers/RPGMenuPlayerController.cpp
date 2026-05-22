// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/RPGMenuPlayerController.h"
#include "UI/Widget/RPGMainMenuWidget.h"
#include "UI/Widget/WidgetLayout_Base.h"
#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "RPGGameplayTags.h"
#include "Engine/GameInstance.h"
#include "Camera/CameraActor.h"
#include "RPGDebugHelper.h"
#include "Kismet/GameplayStatics.h"

ARPGMenuPlayerController::ARPGMenuPlayerController()
{
}

void ARPGMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ACameraActor::StaticClass(), TEXT("MainMenu"), FoundCameras);

	if (!FoundCameras.IsEmpty())
	{
		SetViewTarget(FoundCameras[0]);
	}
	else
	{
		Debug::PrintWarning(TEXT("[MenuPlayerController] BeginPlay - No CameraActor with tag 'MainMenu' found in the level!"));
	}

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	ShowMainMenu();
}

void ARPGMenuPlayerController::ShowMainMenu()
{
	if (!PrimaryGameLayoutClass)
	{
		Debug::PrintWarning(TEXT("[MenuPlayerController] ShowMainMenu - PrimaryGameLayoutClass is null"));
		return;
	}

	if (MainMenuWidgetClass.IsNull())
	{
		Debug::PrintWarning(TEXT("[MenuPlayerController] ShowMainMenu - MainMenuWidgetClass is null"));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		Debug::PrintWarning(TEXT("[MenuPlayerController] ShowMainMenu - GameInstance is null"));
		return;
	}

	URPGUIManagerSubsystem* UIManager = GI->GetSubsystem<URPGUIManagerSubsystem>();
	if (!UIManager)
	{
		Debug::PrintWarning(TEXT("[MenuPlayerController] ShowMainMenu - UIManager is null"));
		return;
	}

	UWidgetLayout_Base* PrimaryLayout = CreateWidget<UWidgetLayout_Base>(this, PrimaryGameLayoutClass);
	if (!PrimaryLayout)
	{
		Debug::PrintError(TEXT("[MenuPlayerController] ShowMainMenu - Failed to create PrimaryGameLayout"));
		return;
	}

	PrimaryLayout->AddToViewport();
	UIManager->RegisterWidgetLayout_Base(PrimaryLayout);

	TSoftClassPtr<URPGWidget_ActivatableBase> SoftMainMenuClass(MainMenuWidgetClass.ToSoftObjectPath());

	UIManager->PushSoftWidgetToStackAsync(
		RPGGameplayTags::RPGCommonUI_WidgetStack_Frontend,
		SoftMainMenuClass,
		[](EAsyncPushWidgetState State, URPGWidget_ActivatableBase* Widget)
		{
		}
	);
}
