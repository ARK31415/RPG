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
