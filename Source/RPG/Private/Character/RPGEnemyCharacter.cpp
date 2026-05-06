// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGEnemyCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Component/Combat/EnemyCombatComponent.h"
#include "DataAsset/StartUpDate/DataAsset_EnemyStartUpData.h"
#include "DataAsset/Character/DataAsset_EnemyConfig.h"
#include "Controllers/RPGEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "RPGGameplayTags.h"
#include "Component/Health/RPGEnemyHealthComponent.h"
#include "Component/UI/RPGEnemyUIComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGEnemyCharacter, Log, All)

ARPGEnemyCharacter::ARPGEnemyCharacter()
{
	// 设置Enemy Tag
	Tags.AddUnique(FName("Enemy"));
	
	// 敌人自动被AI控制器接管（放置在世界或生成时）
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 指定默认AI控制器类
	AIControllerClass = ARPGEnemyAIController::StaticClass();

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,180.f,0.f);
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;

	// Create ability system component on the character itself (for enemies)
	RPGAbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("RPGAbilitySystemComponent"));
	RPGAbilitySystemComponent->SetIsReplicated(true);

	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>("EnemyCombatComponent");

	// Create attribute set
	RPGAttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("RPGAttributeSet"));

	// 创建敌人健康组件（派生类）
	HealthComponent = CreateDefaultSubobject<URPGEnemyHealthComponent>(TEXT("HealthComponent"));

	// 创建敌人 UI 组件
	EnemyUIComponent = CreateDefaultSubobject<URPGEnemyUIComponent>(TEXT("EnemyUIComponent"));
}

UAbilitySystemComponent* ARPGEnemyCharacter::GetAbilitySystemComponent() const
{
	return RPGAbilitySystemComponent;
}

UPawnCombatComponent* ARPGEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}


void ARPGEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ASC with avatar actor (the enemy character itself)
	if (RPGAbilitySystemComponent)
	{
		RPGAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 初始化敌人配置（属性）
	InitializeEnemyConfig();

	// Initialize startup data (grant abilities and effects)
	InitializeStartupData();
}

void ARPGEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 缓存AI控制器并启动行为树
	ARPGEnemyAIController* AIController = Cast<ARPGEnemyAIController>(NewController);
	if (AIController && EnemyBehaviorTree)
	{
		CachedAIController = AIController;
		AIController->RunBehaviorTreeWithBlackboard(EnemyBehaviorTree);

		/*UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[%s] PossessedBy - AI控制器[%s]已接管，行为树[%s]已启动"),
			*GetName(), *AIController->GetName(), *EnemyBehaviorTree->GetName());*/
	}
	else if (!EnemyBehaviorTree)
	{
		UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[%s] PossessedBy - EnemyBehaviorTree 未指定，AI将不会行动"), *GetName());
	}
}

void ARPGEnemyCharacter::InitializeStartupData()
{
	if (!EnemyStartUpData || !RPGAbilitySystemComponent)
	{
		return;
	}

	// Grant abilities and effects from startup data
	EnemyStartUpData->GiveToAbilitySystemComponent(RPGAbilitySystemComponent, 1);
}

void ARPGEnemyCharacter::InitializeEnemyConfig()
{
	if (!EnemyConfig)
	{
		UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[%s] InitializeEnemyConfig - EnemyConfig 为空，无法初始化敌人属性"), *GetName());
		return;
	}

	if (!RPGAbilitySystemComponent)
	{
		UE_LOG(LogRPGEnemyCharacter, Error, TEXT("[%s] InitializeEnemyConfig - ASC 为空"), *GetName());
		return;
	}

	// 应用敌人属性到 ASC
	EnemyConfig->ApplyAttributesToASC(RPGAbilitySystemComponent, 1);

	/*UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[%s] InitializeEnemyConfig - 敌人属性已应用到 ASC, Config=[%s]"),
		*GetName(), *EnemyConfig->GetName());*/
}

void ARPGEnemyCharacter::OnDeathStarted_Implementation()
{
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] OnDeathStarted() called on %s"), *GetName());

	// 敌人特有逻辑（碰撞/移动禁用已由 Death GA 处理）

	// 1. 通知 AI 控制器更新 Blackboard
	if (CachedAIController.IsValid())
	{
		UBlackboardComponent* Blackboard = CachedAIController->GetBlackboardComponent();
		if (Blackboard)
		{
			Blackboard->SetValueAsBool(FName("Dead"), true);
			UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Set Blackboard Dead = true"));
		}
	}

	// 2. 禁用战斗组件
	if (EnemyCombatComponent)
	{
		EnemyCombatComponent->SetComponentTickEnabled(false);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Disabled EnemyCombatComponent"));
	}

	// 3. 死亡动画通过 AnimBlueprint 检测 Shared_Status_Dead Tag 自动播放
}

void ARPGEnemyCharacter::OnDeathFinished_Implementation()
{
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] OnDeathFinished() called on %s"), *GetName());

	// TODO: 敌人死亡后的逻辑
	// 1. 掉落物品
	// 2. 给予经验值
	// 3. 播放音效
	
	// 设置自动销毁时间(3秒)
	SetLifeSpan(3.0f);
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] SetLifeSpan(3.0s), will be destroyed automatically"));
}
