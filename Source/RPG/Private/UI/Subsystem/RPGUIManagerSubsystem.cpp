// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/RPGUIManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.h"
#include "CommonActivatableWidget.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "UI/Widget/WidgetLayout_Base.h"
#include "UI/Widget/RPGWidget_ActivatableBase.h"
#include "UI/Widget/RPGLoadingScreenWidget.h"
#include "Subsystem/RPGLoadingSubsystem.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/RPGHUDWidget.h"
#include "UI/Widget/RPGMainMenuWidget.h"
#include "RPGGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGUIManagerSubsystem, All, All)



URPGUIManagerSubsystem* URPGUIManagerSubsystem::Get(UObject* WorldContextObject)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);

		return  UGameInstance::GetSubsystem<URPGUIManagerSubsystem>(World->GetGameInstance());
	}
	return nullptr;
}

bool URPGUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> FoundClass;
		GetDerivedClasses(GetClass(), FoundClass);
		return FoundClass.IsEmpty();
	}
	return false;
}

void URPGUIManagerSubsystem::RegisterWidgetLayout_Base(UWidgetLayout_Base* InWidget)
{
	check(InWidget);
	CreateWidgetLayout_Base = InWidget;
	UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("RegisterWidgetLayout_Base - Widget registered"));
}

void URPGUIManagerSubsystem::PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag,
	TSoftClassPtr<URPGWidget_ActivatableBase> InSoftWidgetClass,TFunction<void(EAsyncPushWidgetState, URPGWidget_ActivatableBase*)> AsyncPushStateCallback)
{
	check(!InSoftWidgetClass.IsNull());

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, InWidgetStackTag, AsyncPushStateCallback]()
			{
				UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
				
				check(LoadedWidgetClass && CreateWidgetLayout_Base);

				UCommonActivatableWidgetContainerBase* FoundWidgetStack = CreateWidgetLayout_Base->FindWidgetStackByTag(InWidgetStackTag);
				URPGWidget_ActivatableBase* CreatedWidget = FoundWidgetStack->AddWidget<URPGWidget_ActivatableBase>(
					LoadedWidgetClass,
					[AsyncPushStateCallback](URPGWidget_ActivatableBase& CreateWidgetInstance)
					{
						AsyncPushStateCallback(EAsyncPushWidgetState::OnCreatedBeforePush, &CreateWidgetInstance);
					}
				);

				AsyncPushStateCallback(EAsyncPushWidgetState::AfterPush, CreatedWidget);
			}
		)
	);
}

void URPGUIManagerSubsystem::PushToWidgetByTag(TSoftObjectPtr<UWidgetLayout_Base> InWidget, FGameplayTag Tag)
{
}

void URPGUIManagerSubsystem::PushLoadingScreen(TSoftClassPtr<URPGLoadingScreenWidget> LoadingWidgetClass)
{
	if (LoadingWidgetClass.IsNull())
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("PushLoadingScreen - LoadingWidgetClass is null"));
		return;
	}

	// 使用现有的异步推送机制，将 LoadingScreen 推入 Modal 层
	TSoftClassPtr<URPGWidget_ActivatableBase> BaseClass(LoadingWidgetClass.ToSoftObjectPath());

	PushSoftWidgetToStackAsync(
		RPGGameplayTags::RPGCommonUI_WidgetStack_Modal,
		BaseClass,
		[](EAsyncPushWidgetState State, URPGWidget_ActivatableBase* Widget)
		{
			if (State == EAsyncPushWidgetState::AfterPush)
			{
				UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("PushLoadingScreen - LoadingScreen pushed to Modal stack"));
			}
		}
	);
}

void URPGUIManagerSubsystem::PopLoadingScreen()
{
	if (!CreateWidgetLayout_Base)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("PopLoadingScreen - CreateWidgetLayout_Base is null"));
		return;
	}

	UCommonActivatableWidgetContainerBase* ModalStack = CreateWidgetLayout_Base->FindWidgetStackByTag(
		RPGGameplayTags::RPGCommonUI_WidgetStack_Modal);

	if (!ModalStack)
	{
		UE_LOG(LogRPGUIManagerSubsystem, Warning, TEXT("PopLoadingScreen - Modal stack not found"));
		return;
	}

	// 获取栈顶活跃 Widget，检查是否为 LoadingScreen 类型
	UCommonActivatableWidget* ActiveWidget = ModalStack->GetActiveWidget();
	if (ActiveWidget && ActiveWidget->IsA<URPGLoadingScreenWidget>())
	{
		ActiveWidget->DeactivateWidget();
		UE_LOG(LogRPGUIManagerSubsystem, Log, TEXT("PopLoadingScreen - LoadingScreen popped from Modal stack"));
	}
}
