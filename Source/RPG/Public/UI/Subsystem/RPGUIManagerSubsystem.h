// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameUIManagerSubsystem.h"
#include "Containers/Ticker.h"
#include "RPGUIManagerSubsystem.generated.h"

class URPGWidget_ActivatableBase;
class URPGLoadingScreenWidget;
class UWidgetLayout_Base;
class APlayerController;
class UPrimaryGameLayout;

enum class EAsyncPushWidgetState : uint8
{
	OnCreatedBeforePush,
	AfterPush
};

/**
 * RPG 项目的 UI 管理器 Subsystem。
 * 负责管理 CommonUI 四层栈（Modal → Menu → GameMenu → Game）的 Widget 生命周期。
 * 生命周期跟随 GameInstance，全局唯一。
 */
UCLASS()
class RPG_API URPGUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	static URPGUIManagerSubsystem* Get(UObject* WorldContextObject);

	// begin USubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// end USubsystem Interface

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterWidgetLayout_Base(UWidgetLayout_Base* InWidget);

	void PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag,
	                                TSoftClassPtr<URPGWidget_ActivatableBase> InSoftWidgetClass,
	                                TFunction<void(EAsyncPushWidgetState, URPGWidget_ActivatableBase*)>
	                                AsyncPushStateCallback);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PushToWidgetByTag(TSoftObjectPtr<UWidgetLayout_Base> InWidget, FGameplayTag Tag);

	/**
	 * 将 LoadingScreen Widget 推入 Modal 栈（用于流式加载进度展示）
	 * @param LoadingWidgetClass LoadingScreen Widget 的软引用类（蓝图子类）
	 */
	void PushLoadingScreen(TSoftClassPtr<URPGLoadingScreenWidget> LoadingWidgetClass);

	/** 从 Modal 栈弹出 LoadingScreen Widget */
	void PopLoadingScreen();

private:
	UPROPERTY(Transient)
	UWidgetLayout_Base* CreateWidgetLayout_Base;
};
