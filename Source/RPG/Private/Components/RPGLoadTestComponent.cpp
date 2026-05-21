// RPGLoadTestComponent - 关卡加载性能测试组件实现

#include "Components/RPGLoadTestComponent.h"
#include "Subsystem/RPGLoadingSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "RPGDebugHelper.h"

URPGLoadTestComponent::URPGLoadTestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URPGLoadTestComponent::RunSyncLoadTest()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		Debug::PrintError(TEXT("[LoadTest] RunSyncLoadTest - GameInstance is null"));
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		Debug::PrintError(TEXT("[LoadTest] RunSyncLoadTest - LoadingSubsystem is null"));
		return;
	}

	Debug::PrintWarning(TEXT("[LoadTest] ========== Starting SYNC Load Test =========="));

	// 返回到测试起始地图（假设为第一个）
	if (TestMaps.Num() > 0)
	{
		const FString StartMap = TestMaps[0].ToSoftObjectPath().ToString();
		UGameplayStatics::OpenLevel(this, FName(*StartMap));
		
		Debug::PrintWarning(FString::Printf(TEXT("[LoadTest] Returned to start map: %s"), *StartMap));
		Debug::PrintWarning(TEXT("[LoadTest] Now manually call SyncLoadLevel for each test map"));
	}
}

void URPGLoadTestComponent::RunAsyncLoadTest()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		Debug::PrintError(TEXT("[LoadTest] RunAsyncLoadTest - GameInstance is null"));
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		Debug::PrintError(TEXT("[LoadTest] RunAsyncLoadTest - LoadingSubsystem is null"));
		return;
	}

	Debug::PrintWarning(TEXT("[LoadTest] ========== Starting ASYNC Load Test =========="));

	// 异步加载测试地图
	for (const auto& TestMap : TestMaps)
	{
		if (!TestMap.IsNull())
		{
			const FString MapPath = TestMap.ToSoftObjectPath().ToString();
			Debug::PrintWarning(FString::Printf(TEXT("[LoadTest] Async loading: %s"), *MapPath));
			LoadingSubsystem->AsyncLoadLevel(TestMap);
		}
	}
}

void URPGLoadTestComponent::PrintTestReport() const
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		Debug::PrintError(TEXT("[LoadTest] PrintTestReport - GameInstance is null"));
		return;
	}

	const URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		Debug::PrintError(TEXT("[LoadTest] PrintTestReport - LoadingSubsystem is null"));
		return;
	}

	LoadingSubsystem->PrintPerformanceReport();
}
