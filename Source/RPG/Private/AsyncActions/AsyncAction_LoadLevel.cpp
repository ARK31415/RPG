// AsyncAction_LoadLevel - 蓝图异步加载关卡节点实现

#include "AsyncActions/AsyncAction_LoadLevel.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystem/RPGLoadingSubsystem.h"

UAsyncAction_LoadLevel* UAsyncAction_LoadLevel::AsyncLoadLevel(const UObject* WorldContextObject,
                                                                TSoftObjectPtr<UWorld> TargetLevel,
                                                                FString Options)
{
	if (!TargetLevel.IsNull() && GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			UAsyncAction_LoadLevel* Node = NewObject<UAsyncAction_LoadLevel>();
			Node->CachedWorld = World;
			Node->CachedTargetLevel = TargetLevel;
			Node->CachedOptions = Options;

			Node->RegisterWithGameInstance(World);

			return Node;
		}
	}

	return nullptr;
}

void UAsyncAction_LoadLevel::Activate()
{
	UWorld* World = CachedWorld.Get();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		SetReadyToDestroy();
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		SetReadyToDestroy();
		return;
	}

	// 订阅进度和状态委托
	LoadingSubsystem->OnLoadingProgressChanged.AddDynamic(this, &UAsyncAction_LoadLevel::HandleProgressChanged);
	LoadingSubsystem->OnLoadingStateChanged.AddDynamic(this, &UAsyncAction_LoadLevel::HandleStateChanged);

	// 触发加载
	LoadingSubsystem->AsyncLoadLevel(CachedTargetLevel, true, CachedOptions);
}

void UAsyncAction_LoadLevel::HandleProgressChanged(const FRPGLoadingProgress& Progress)
{
	if (!bHasStarted)
	{
		// 首次进度回调视为加载开始
		bHasStarted = true;
		OnLoadStarted.Broadcast(Progress);
	}

	// 广播进度更新
	OnProgressUpdated.Broadcast(Progress);
}

void UAsyncAction_LoadLevel::HandleStateChanged(ERPGLoadingState NewState)
{
	if (NewState == ERPGLoadingState::Complete || NewState == ERPGLoadingState::Idle)
	{
		// 加载完成，广播最终进度
		if (UWorld* World = CachedWorld.Get())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (URPGLoadingSubsystem* LS = GI->GetSubsystem<URPGLoadingSubsystem>())
				{
					// 取消订阅
					LS->OnLoadingProgressChanged.RemoveDynamic(this, &UAsyncAction_LoadLevel::HandleProgressChanged);
					LS->OnLoadingStateChanged.RemoveDynamic(this, &UAsyncAction_LoadLevel::HandleStateChanged);

					// 广播完成
					OnLoadCompleted.Broadcast(LS->GetCurrentProgress());
				}
			}
		}

		SetReadyToDestroy();
	}
}
