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
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Constructor called, bIsFocusable set to true"));
}

bool URPGMainMenuWidget::Initialize()
{
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Initialize() called"));

	if (!Super::Initialize())
	{
		UE_LOG(LogRPGMainMenuWidget, Error, TEXT("[MainMenu] Initialize() - Super::Initialize() returned false!"));
		return false;
	}

	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Initialize() - Super init OK"));

	FString TitleName = TitleText ? TEXT("Found") : TEXT("NULL");
	FString StartBtnName = StartGameButton ? TEXT("Found") : TEXT("NULL");
	FString ExitBtnName = ExitGameButton ? TEXT("Found") : TEXT("NULL");

	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Initialize() - TitleText=%s, StartGameButton=%s, ExitGameButton=%s"),
		*TitleName, *StartBtnName, *ExitBtnName);

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnStartGameClicked);
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Initialize() - StartGameButton.OnClicked bound"));
	}

	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &URPGMainMenuWidget::OnExitGameClicked);
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] Initialize() - ExitGameButton.OnClicked bound"));
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
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnActivated() called"));

	APlayerController* PC = GetOwningPlayer();
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnActivated() - OwningPlayer=%s"),
		PC ? *PC->GetName() : TEXT("NULL"));

	SetFocus();

	if (StartGameButton)
	{
		StartGameButton->SetIsEnabled(true);
		StartGameButton->SetKeyboardFocus();
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnActivated() - StartGameButton enabled, keyboard focus set"));
	}

	if (ExitGameButton)
	{
		ExitGameButton->SetIsEnabled(true);
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnActivated() - ExitGameButton enabled"));
	}
}

void URPGMainMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnDeactivated() called"));

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
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnKeyDown - Key=%s"), *Key.ToString());

	if (Key == EKeys::S || Key == EKeys::Enter)
	{
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnKeyDown - KEY TEST: Starting game via keyboard"));
		StartGame();
		return FReply::Handled();
	}

	if (Key == EKeys::Escape || Key == EKeys::Q)
	{
		UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnKeyDown - KEY TEST: Exiting game via keyboard"));
		ExitGame();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply URPGMainMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] NativeOnMouseButtonDown - Mouse button=%s"), *InMouseEvent.GetEffectingButton().ToString());
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URPGMainMenuWidget::StartGame()
{
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] StartGame() called"));

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

	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] StartGame - LoadingSubsystem found, starting async load"));

	TSoftObjectPtr<UWorld> MainLevel(FSoftObjectPath(TEXT("/Game/ThirdPerson/Lvl_ThirdPerson")));
	LoadingSubsystem->AsyncLoadLevel(MainLevel, true, TEXT("?listen"));
}

void URPGMainMenuWidget::ExitGame()
{
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] ExitGame() called"));

	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
#if WITH_EDITOR
		if (!IsRunningGame() && GetWorld() && GetWorld()->IsPlayInEditor())
		{
			UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] ExitGame - PIE detected, using ConsoleCommand quit"));
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
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] OnStartGameClicked() - Button click received!"));
	StartGame();
}

void URPGMainMenuWidget::OnExitGameClicked()
{
	UE_LOG(LogRPGMainMenuWidget, Log, TEXT("[MainMenu] OnExitGameClicked() - Button click received!"));
	ExitGame();
}
