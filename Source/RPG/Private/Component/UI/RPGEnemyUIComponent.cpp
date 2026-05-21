// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/UI/RPGEnemyUIComponent.h"
#include "Component/Health/RPGEnemyHealthComponent.h"
#include "Character/RPGEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/RPGEnemyHealthBarWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyUIComponent, Log, All)

URPGEnemyUIComponent::URPGEnemyUIComponent()
{
	MaxHealthBarDistance = 2000.0f;
	bHideHealthBarWhenFull = false;
	DisplayName = TEXT("Enemy");
	EnemyLevel = 1;
}

URPGHealthComponent* URPGEnemyUIComponent::GetHealthComponentInternal() const
{
	// 从 Enemy Character 获取 EnemyHealthComponent
	if (const ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(GetOwner()))
	{
		return EnemyCharacter->GetHealthComponent();
	}
	return nullptr;
}

void URPGEnemyUIComponent::BeginPlay()
{
	Super::BeginPlay();

	// 从 Owner 查找 WidgetComponent
	if (AActor* Owner = GetOwner())
	{
		CachedHealthBarWidgetComponent = Owner->FindComponentByClass<UWidgetComponent>();
	}



	// 初始化血条 Widget
	InitializeHealthBarWidget();

	UE_LOG(LogRPGEnemyUIComponent, Log, TEXT("EnemyUIComponent initialized for %s (Level %d)"), 
		*GetOwner()->GetName(), EnemyLevel);
}

void URPGEnemyUIComponent::OnHealthChangedInternal(float NewHealth, float OldHealth)
{
	// 调用基类实现（广播给 UI）
	Super::OnHealthChangedInternal(NewHealth, OldHealth);

	// ViewModel 职责：根据血量状态控制 WidgetComponent 可见性
	if (CachedHealthBarWidgetComponent.IsValid())
	{
		CachedHealthBarWidgetComponent->SetVisibility(ShouldShowHealthBar());
	}
}

bool URPGEnemyUIComponent::ShouldShowHealthBar() const
{
	if (!HealthComponent)
	{
		return false;
	}

	// 死亡时隐藏
	if (HealthComponent->IsDead())
	{
		return false;
	}

	// 满血时隐藏
	if (bHideHealthBarWhenFull && HealthComponent->GetCurrentHealth() >= HealthComponent->GetMaxHealth())
	{
		return false;
	}

	// 距离检测
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
		if (Distance > MaxHealthBarDistance)
		{
			return false;
		}
	}

	return true;
}

FString URPGEnemyUIComponent::GetEnemyDisplayName() const
{
	return DisplayName;
}

int32 URPGEnemyUIComponent::GetEnemyLevel() const
{
	return EnemyLevel;
}

void URPGEnemyUIComponent::InitializeHealthBarWidget()
{
	UE_LOG(LogRPGEnemyUIComponent, Warning,
		TEXT("[%s] InitializeHealthBarWidget: ENTRY - WidgetComp=%s, HealthBarWidgetClass=%s, CharClass=%s"),
		*GetOwner()->GetName(),
		CachedHealthBarWidgetComponent.IsValid() ? TEXT("Valid") : TEXT("NULL"),
		HealthBarWidgetClass ? *HealthBarWidgetClass->GetName() : TEXT("NULL"),
		(GetOwner() && GetOwner()->GetClass()) ? *GetOwner()->GetClass()->GetName() : TEXT("NULL"));

	if (!CachedHealthBarWidgetComponent.IsValid())
	{
		UE_LOG(LogRPGEnemyUIComponent, Warning, 
			TEXT("[%s] InitializeHealthBarWidget: WidgetComponent not found"), *GetOwner()->GetName());
		return;
	}

	UUserWidget* Widget = CachedHealthBarWidgetComponent->GetUserWidgetObject();
	if (!Widget)
	{
		TSubclassOf<UUserWidget> ClassToUse = HealthBarWidgetClass;

		if (!ClassToUse)
		{
			if (const ARPGEnemyCharacter* EnemyCharacter = Cast<ARPGEnemyCharacter>(GetOwner()))
			{
				ClassToUse = EnemyCharacter->HealthBarWidgetClass;
			}
		}

		if (ClassToUse)
		{
			CachedHealthBarWidgetComponent->SetWidgetClass(ClassToUse);
			CachedHealthBarWidgetComponent->InitWidget();
			Widget = CachedHealthBarWidgetComponent->GetUserWidgetObject();
		}
	}

	if (!Widget)
	{
		UE_LOG(LogRPGEnemyUIComponent, Warning, 
			TEXT("[%s] InitializeHealthBarWidget: Widget Class not set on WidgetComponent"), *GetOwner()->GetName());
		return;
	}

	URPGEnemyHealthBarWidget* HealthBarWidget = Cast<URPGEnemyHealthBarWidget>(Widget);
	if (!HealthBarWidget)
	{
		UE_LOG(LogRPGEnemyUIComponent, Error, 
			TEXT("[%s] InitializeHealthBarWidget: Widget is not URPGEnemyHealthBarWidget"), *GetOwner()->GetName());
		return;
	}

	// View 通过 ViewModel 接口绑定
	HealthBarWidget->InitEnemyCreatedWidget(GetOwner());

	// ViewModel 控制初始可见性
	CachedHealthBarWidgetComponent->SetVisibility(ShouldShowHealthBar());

	UE_LOG(LogRPGEnemyUIComponent, Log, 
		TEXT("[%s] InitializeHealthBarWidget: Health bar widget initialized"), *GetOwner()->GetName());
}

void URPGEnemyUIComponent::ResetHealthBarWidget()
{
	// 对象池重新激活时调用
	InitializeHealthBarWidget();
}
