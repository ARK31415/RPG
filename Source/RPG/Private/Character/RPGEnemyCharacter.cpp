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
#include "Animation/AnimationInstances/Enemy/RPGEnemyAnimInstanceBase.h"
#include "RPGGameplayTags.h"
#include "Component/Health/RPGEnemyHealthComponent.h"
#include "Component/UI/RPGEnemyUIComponent.h"
#include "Subsystem/RPGEnemyPoolSubsystem.h"
#include "RPGDebugHelper.h"

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
	// 职责：仅处理"立即禁用交互"表现逻辑（碰撞/移动/AI/战斗）
	// 注意：不能在此处设置 bPauseAnims=true，否则后续的死亡 Montage 无法推进。
	// 动画姿势锁定交由 OnDeathFinished 处理。
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] OnDeathStarted() called on %s"), *GetName());
	Debug::Print(FString::Printf(TEXT("[Death] Enemy::OnDeathStarted - %s, IsHidden=%s"), *GetName(), IsHidden() ? TEXT("true") : TEXT("false")));


	// 1. 禁用胶囊体碰撞
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Disabled capsule collision"));
	}

	// 2. 停止移动组件
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->SetMovementMode(MOVE_None);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Stopped movement component"));
	}

	// 3. 停止 AI 行为树
	if (CachedAIController.IsValid())
	{
		UBlackboardComponent* Blackboard = CachedAIController->GetBlackboardComponent();
		if (Blackboard)
		{
			Blackboard->SetValueAsBool(FName("Dead"), true);
			UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Set Blackboard Dead = true"));
		}

		if (UBrainComponent* BrainComp = CachedAIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Death"));
			UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Stopped AI behavior tree"));
		}
	}

	// 4. 禁用战斗组件
	if (EnemyCombatComponent)
	{
		EnemyCombatComponent->SetComponentTickEnabled(false);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Disabled EnemyCombatComponent"));
	}
}

void ARPGEnemyCharacter::OnDeathFinished_Implementation()
{
	// 职责：Montage 已播完，锁定姿势 + 回收/销毁
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] OnDeathFinished() called on %s"), *GetName());
	Debug::Print(FString::Printf(TEXT("[Death] Enemy::OnDeathFinished - %s, IsHidden=%s"), *GetName(), IsHidden() ? TEXT("true") : TEXT("false")));

	// 1. 锁定死亡姿势：Montage 已播完，此时暂停 Mesh 动画作为防御措施
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->bPauseAnims = true;
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Locked death pose (bPauseAnims = true)"));
		Debug::Print(FString::Printf(TEXT("[Death] Lock Pose - %s, bPauseAnims=true"), *GetName()));
	}

	// TODO: 敌人死亡后的逻辑
	// 1. 掉落物品
	// 2. 给予经验值
	// 3. 播放音效


	// 根据 Tag 判断敌人来源，决定回收方式
	if (UWorld* World = GetWorld())
	{
		// 检查是否来自对象池
		if (Tags.Contains(FName("SpawnedFromPool")))
		{
			if (URPGEnemyPoolSubsystem* Pool = World->GetSubsystem<URPGEnemyPoolSubsystem>())
			{
				Pool->ReleaseEnemy(this);
				UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Released to pool: %s"), *GetName());
				Debug::Print(FString::Printf(TEXT("[Death] Enemy Released to Pool - %s"), *GetName()));
				return;
			}
		}
	}

	// 回退：Pool 不可用或普通生成的敌人，直接销毁
	SetLifeSpan(0.1f);
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] No pool or direct spawn, SetLifeSpan(0.1s): %s"), *GetName());
	Debug::Print(FString::Printf(TEXT("[Death] Enemy SetLifeSpan(0.1s) -> Destroy - %s"), *GetName()));
}
