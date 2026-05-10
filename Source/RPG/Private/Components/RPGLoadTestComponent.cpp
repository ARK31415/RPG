// RPGLoadTestComponent - 关卡加载性能测试组件实现

#include "Components/RPGLoadTestComponent.h"
#include "Subsystem/RPGLoadingSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

URPGLoadTestComponent::URPGLoadTestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URPGLoadTestComponent::RunSyncLoadTest()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("RunSyncLoadTest - GameInstance is null"));
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("RunSyncLoadTest - LoadingSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("========== Starting SYNC Load Test =========="));

	// 返回到测试起始地图（假设为第一个）
	if (TestMaps.Num() > 0)
	{
		const FString StartMap = TestMaps[0].ToSoftObjectPath().ToString();
		UGameplayStatics::OpenLevel(this, FName(*StartMap));
		
		UE_LOG(LogTemp, Warning, TEXT("Returned to start map: %s"), *StartMap);
		UE_LOG(LogTemp, Warning, TEXT("Now manually call SyncLoadLevel for each test map"));
	}
}

void URPGLoadTestComponent::RunAsyncLoadTest()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("RunAsyncLoadTest - GameInstance is null"));
		return;
	}

	URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("RunAsyncLoadTest - LoadingSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("========== Starting ASYNC Load Test =========="));

	// 异步加载测试地图
	for (const auto& TestMap : TestMaps)
	{
		if (!TestMap.IsNull())
		{
			const FString MapPath = TestMap.ToSoftObjectPath().ToString();
			UE_LOG(LogTemp, Warning, TEXT("Async loading: %s"), *MapPath);
			LoadingSubsystem->AsyncLoadLevel(TestMap);
		}
	}
}

void URPGLoadTestComponent::PrintTestReport() const
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("PrintTestReport - GameInstance is null"));
		return;
	}

	const URPGLoadingSubsystem* LoadingSubsystem = GI->GetSubsystem<URPGLoadingSubsystem>();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("PrintTestReport - LoadingSubsystem is null"));
		return;
	}

	LoadingSubsystem->PrintPerformanceReport();
}
