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

		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[%s] PossessedBy - AI控制器[%s]已接管，行为树[%s]已启动"),
			*GetName(), *AIController->GetName(), *EnemyBehaviorTree->GetName());
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

	UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[%s] InitializeEnemyConfig - 敌人属性已应用到 ASC, Config=[%s]"),
		*GetName(), *EnemyConfig->GetName());
}

void ARPGEnemyCharacter::Die()
{
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] Die() called on %s"), *GetName());

	// 1. 设置死亡Tag
	if (RPGAbilitySystemComponent)
	{
		RPGAbilitySystemComponent->AddLooseGameplayTag(RPGGameplayTags::Shared_Status_Dead);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Set Shared_Status_Dead tag"));
	}

	// 2. 通知AI控制器更新Blackboard
	if (CachedAIController.IsValid())
	{
		UBlackboardComponent* Blackboard = CachedAIController->GetBlackboardComponent();
		if (Blackboard)
		{
			Blackboard->SetValueAsBool(FName("Dead"), true);
			UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Set Blackboard Dead = true"));
		}
	}

	// 3. 禁用战斗组件
	if (EnemyCombatComponent)
	{
		EnemyCombatComponent->SetComponentTickEnabled(false);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Disabled EnemyCombatComponent"));
	}

	// 4. 禁用碰撞盒
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Disabled collision"));
	}

	// 5. 停止移动
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_None);
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Stopped movement"));
	}

	// 6. 播放死亡动画(通过动画蓝图检测Tag)
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UE_LOG(LogRPGEnemyCharacter, Log, TEXT("[Enemy] Playing death animation via AnimBlueprint"));
	}

	// 7. 设置自动销毁时间(5秒)
	SetLifeSpan(5.0f);
	UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] SetLifeSpan(5.0s), will be destroyed automatically"));
}
