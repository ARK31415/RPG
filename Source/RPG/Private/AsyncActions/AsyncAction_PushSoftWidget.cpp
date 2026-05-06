// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncActions/AsyncAction_PushSoftWidget.h"

#include "PrimaryGameLayout.h"
#include "UI/Widget/RPGWidget_ActivatableBase.h"
#include "UI/Subsystem/RPGUIManagerSubsystem.h"

UAsyncAction_PushSoftWidget* UAsyncAction_PushSoftWidget::PushSoftWidget(const UObject* WorldContextObject, APlayerController* OwningPlayerController, TSoftClassPtr<URPGWidget_ActivatableBase> InSoftWidgetClass, UPARAM(meta = (Categories = "RPGCommonUI.WidgetStack")) FGameplayTag InWidgetStackTag, bool bFocusOnNewlyPushedWidget)
{	
	checkf(!InSoftWidgetClass.IsNull(),TEXT("PushSoftWidgetToStack was passed a null soft widget class "));

	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull))
		{
			UAsyncAction_PushSoftWidget* Node = NewObject<UAsyncAction_PushSoftWidget>();
			Node->CachedOwningWorld = World;
			Node->CachedOwningPlayerController = OwningPlayerController;
			Node->CachedSoftWidgetClass = InSoftWidgetClass;
			Node->CachedWidgetStackTag = InWidgetStackTag;
			Node->bCachedFocusOnNewlyPushWidget = bFocusOnNewlyPushedWidget;

			Node->RegisterWithGameInstance(World);

			return Node;
		}
	}

	return nullptr;
}

void UAsyncAction_PushSoftWidget::Activate()
{
	URPGUIManagerSubsystem* RPGUIManagerSubsystem = URPGUIManagerSubsystem::Get(CachedOwningWorld.Get());

	RPGUIManagerSubsystem->PushSoftWidgetToStackAsync(CachedWidgetStackTag, CachedSoftWidgetClass,
		[this](EAsyncPushWidgetState InPushState, URPGWidget_ActivatableBase* PushedWidget)
		{
			switch (InPushState)
			{
			case EAsyncPushWidgetState::OnCreatedBeforePush:

				PushedWidget->SetOwningPlayer(CachedOwningPlayerController.Get());

				OnWidgetCreatedBeforePush.Broadcast(PushedWidget);
				
				break;
			case EAsyncPushWidgetState::AfterPush:

				OnAfterPush.Broadcast(PushedWidget);

				if (bCachedFocusOnNewlyPushWidget)
				{
					if (UWidget* WidgetToFocus = PushedWidget->GetDesiredFocusWidget())
					{
						WidgetToFocus->SetFocus();
					}
				}

				SetReadyToDestroy();
				
				break;
			default:
				break;
			}
		}
	);
}
