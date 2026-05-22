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
	bIsFocusable = true;
}

bool URPGMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		UE_LOG(LogRPGMainMenuWidget, Error, TEXT("[MainMenu] Initialize() - Super::Initialize() returned false!"));
		return false;
	}

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnStartGameClicked);
	}

	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnExitGameClicked);
	}

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("RPG Game")));
	}

	return true;
}

void URPGMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetFocus();

	if (StartGameButton)
	{
		StartGameButton->SetIsEnabled(true);
		StartGameButton->SetKeyboardFocus();
	}

	if (ExitGameButton)
	{
		ExitGameButton->SetIsEnabled(true);
	}
}

void URPGMainMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.Clear();
	}

	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.Clear();
	}
}

FReply URPGMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::S || Key == EKeys::Enter)
	{
		StartGame();
		return FReply::Handled();
	}

	if (Key == EKeys::Escape || Key == EKeys::Q)
	{
		ExitGame();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply URPGMainMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URPGMainMenuWidget::StartGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("[MainMenu] StartGame - World is null"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("[MainMenu] StartGame - GameInstance is null"));
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("[MainMenu] StartGame - LoadingSubsystem is null"));
		return;
	}

	TSoftObjectPtr<UWorld> MainLevel(FSoftObjectPath(TEXT("/Game/ThirdPerson/Lvl_ThirdPerson")));
	LoadingSubsystem->AsyncLoadLevel(MainLevel, true, TEXT("?listen"));
}

void URPGMainMenuWidget::ExitGame()
{
	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
#if WITH_EDITOR
		if (!IsRunningGame() && GetWorld() && GetWorld()->IsPlayInEditor())
		{
			PC->ConsoleCommand("quit");
			return;
		}
#endif
		UKismetSystemLibrary::QuitGame(PC, nullptr, EQuitPreference::Quit, false);
	}
	else
	{
		UE_LOG(LogRPGMainMenuWidget, Warning, TEXT("[MainMenu] ExitGame - PlayerController is null"));
	}
}

void URPGMainMenuWidget::OnStartGameClicked()
{
	StartGame();
}

void URPGMainMenuWidget::OnExitGameClicked()
{
	ExitGame();
}
