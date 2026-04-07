// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGPlayerCharacter.h"

#include "EnhancedInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RPGGameplayTags.h"
#include "Animation/AnimationInstances/RPGCharacterAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Component/Combat/PlayerCombatComponent.h"
#include "Component/Input/RPGEnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimationInstances/RPGItemAnimLayersBase.h"
#include "Items/Weapon/RPGPlayerWeapon.h"
#include "Character/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPGPlayerCharacter, All, All)

ARPGPlayerCharacter::ARPGPlayerCharacter()
	: BaseTurnSpeed(180.f)
	, MaxTurnSpeed(720.f)
	, SpeedTurnMultiplier(1.0f)
	, AngleTurnMultiplier(0.5f)
	, CurrentWeaponType(ERPGWeaponType::None)
	, bHasMovementInput(false)
	, bShowRotationDebug(false)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	CameraBoom->bUsePawnControlRotation = true; //使用人物控制

	//摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 启用根运动支持
	GetCharacterMovement()->bEnablePhysicsInteraction = true; 
	// 禁用自动朝向移动，改用手动控制转向
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 200.f;

	// 创建战斗组件
	PlayerCombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("PlayerCombatComponent"));
}

UAbilitySystemComponent* ARPGPlayerCharacter::GetAbilitySystemComponent() const
{
	if (ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void ARPGPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ARPGPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void ARPGPlayerCharacter::InitAbilityActorInfo()
{
	ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>();
	if (!PS)
	{
		return;
	}

	if (URPGAbilitySystemComponent* ASC = PS->GetRPGAbilitySystemComponent())
	{
		// OwnerActor = PlayerState, AvatarActor = this (Character)
		ASC->InitAbilityActorInfo(PS, this);
		
		// 缓存到 Character 的成员变量
		AbilitySystemComponent = ASC;
		AttributeSet = PS->GetRPGAttributeSet();
		
		UE_LOG(LogRPGPlayerCharacter, Log, 
			TEXT("ARPGPlayerCharacter::InitAbilityActorInfo - ASC and AttributeSet initialized successfully!"));
		UE_LOG(LogRPGPlayerCharacter, Log, 
			TEXT("  - ASC: %s"), 
			*ASC->GetClass()->GetName());
		UE_LOG(LogRPGPlayerCharacter, Log, 
			TEXT("  - AttributeSet: %s"), 
			AttributeSet ? *AttributeSet->GetClass()->GetName() : TEXT("None"));
	}
}

void ARPGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 缓存动画实例
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		UAnimInstance* BaseAnimInstance = MeshComp->GetAnimInstance();
		
		// 打印基础动画实例信息
		if (BaseAnimInstance)
		{
			UE_LOG(LogRPGPlayerCharacter, Log, 
				TEXT("ARPGPlayerCharacter::BeginPlay - Mesh AnimInstance Class: %s"), 
				*BaseAnimInstance->GetClass()->GetName());
			
			// 检查父类
			/*UClass* ParentClass = BaseAnimInstance->GetClass()->GetSuperClass();
			if (ParentClass)
			{
				UE_LOG(LogRPGPlayerCharacter, Log, 
					TEXT("ARPGPlayerCharacter::BeginPlay - Parent Class: %s"), 
					*ParentClass->GetName());
			}*/
		}
		
		CachedAnimInstance = Cast<URPGCharacterAnimInstance>(BaseAnimInstance);
		
		// 打印动画实例信息
		/*if (CachedAnimInstance)
		{
			UE_LOG(LogRPGPlayerCharacter, Log, 
				TEXT("ARPGPlayerCharacter::BeginPlay - Cached AnimInstance: %s"), 
				*CachedAnimInstance->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogRPGPlayerCharacter, Warning, 
				TEXT("ARPGPlayerCharacter::BeginPlay - Failed to cast to URPGCharacterAnimInstance!"));
			UE_LOG(LogRPGPlayerCharacter, Warning, 
				TEXT("Please ensure ABP_Mannequin_Base's Parent Class is set to URPGCharacterAnimInstance"));
		}*/
	}
	else
	{
		UE_LOG(LogRPGPlayerCharacter, Error, TEXT("ARPGPlayerCharacter::BeginPlay - Mesh is null!"));
	}

	// 初始化角色配置
	InitializeCharacterConfig();

	// 初始化目标旋转为当前朝向
	TargetRotation = GetActorRotation();
}

void ARPGPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 只有在有移动输入时才进行平滑转向
	if (bHasMovementInput)
	{
		SmoothRotateToTarget(DeltaTime);
	}

	// 绘制调试射线
	if (bShowRotationDebug)
	{
		DrawRotationDebug();
	}
}

void ARPGPlayerCharacter::SmoothRotateToTarget(float DeltaTime)
{
	const FRotator CurrentRotation = GetActorRotation();
	
	// 计算动态转向速度
	const float DynamicTurnSpeed = CalculateDynamicTurnSpeed();
	
	// 使用 RInterpTo 平滑插值到目标旋转
	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		DynamicTurnSpeed / 60.f  // 将度/秒转换为插值速度
	);

	// 只更新 Yaw，保持 Pitch 和 Roll 不变
	SetActorRotation(FRotator(CurrentRotation.Pitch, NewRotation.Yaw, CurrentRotation.Roll));
}

float ARPGPlayerCharacter::CalculateDynamicTurnSpeed() const
{
	// 获取当前移动速度
	const float CurrentSpeed = GetVelocity().Size2D();
	const float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	
	// 速度比例 (0-1)
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.f, 1.f);
	
	// 计算当前朝向与目标朝向的角度差
	const float YawDelta = FMath::Abs(FRotator::NormalizeAxis(TargetRotation.Yaw - GetActorRotation().Yaw));
	
	// 角度比例 (0-1)，180度为最大
	const float AngleRatio = FMath::Clamp(YawDelta / 180.f, 0.f, 1.f);
	
	// 动态计算转向速度
	// 基础速度 + 速度贡献 + 角度贡献
	const float SpeedBonus = (MaxTurnSpeed - BaseTurnSpeed) * SpeedRatio * SpeedTurnMultiplier;
	const float AngleBonus = (MaxTurnSpeed - BaseTurnSpeed) * AngleRatio * AngleTurnMultiplier;
	
	// 取两者中的较大值作为额外加成，避免重复叠加
	const float TotalBonus = FMath::Max(SpeedBonus, AngleBonus);
	
	return FMath::Clamp(BaseTurnSpeed + TotalBonus, BaseTurnSpeed, MaxTurnSpeed);
}

void ARPGPlayerCharacter::DrawRotationDebug()
{
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f); // 稍微抬高以便观察
	const float LineLength = 150.f;

	// 绿色射线：当前朝向
	const FVector CurrentForward = GetActorForwardVector();
	const FVector CurrentEnd = Start + CurrentForward * LineLength;
	DrawDebugLine(GetWorld(), Start, CurrentEnd, FColor::Green, false, -1.f, 0, 3.f);

	// 红色射线：目标朝向
	const FVector TargetForward = TargetRotation.Vector();
	const FVector TargetEnd = Start + TargetForward * LineLength;
	DrawDebugLine(GetWorld(), Start, TargetEnd, FColor::Red, false, -1.f, 0, 3.f);
}

void ARPGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(InputConfigDataAsset, TEXT("InputConfig is null"))

	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	checkf(InputSubsystem, TEXT("InputSubsystem is null"))
	InputSubsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);

	URPGEnhancedInputComponent* RPGInputComponent = CastChecked<URPGEnhancedInputComponent>(PlayerInputComponent);
	RPGInputComponent->BindNativeInputAction(InputConfigDataAsset, RPGGameplayTags::InputTag_Move,
	                                         ETriggerEvent::Triggered, this,
	                                         &ThisClass::Input_Move);
	RPGInputComponent->BindNativeInputAction(InputConfigDataAsset, RPGGameplayTags::InputTag_Look,
	                                         ETriggerEvent::Triggered, this,
	                                         &ThisClass::Input_Look);

	 RPGInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed,
	 											  &ThisClass::Input_AbilityInputReleased);
}

void ARPGPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2d MovementVector = InputActionValue.Get<FVector2d>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	// 检查是否有有效输入
	bHasMovementInput = !MovementVector.IsNearlyZero();

	if (bHasMovementInput)
	{
		// 计算输入方向
		FVector InputDirection = FVector::ZeroVector;
		
		if (MovementVector.Y != 0.f)
		{
			InputDirection += MovementRotation.RotateVector(FVector::ForwardVector) * MovementVector.Y;
		}
		if (MovementVector.X != 0.f)
		{
			InputDirection += MovementRotation.RotateVector(FVector::RightVector) * MovementVector.X;
		}

		// 设置目标旋转为输入方向
		if (!InputDirection.IsNearlyZero())
		{
			TargetRotation = InputDirection.Rotation();
		}

		// 添加移动输入
		if (MovementVector.Y != 0.f)
		{
			const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(ForwardDirection, MovementVector.Y);
		}
		if (MovementVector.X != 0.f)
		{
			const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void ARPGPlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2d LookAxisVector = InputActionValue.Get<FVector2d>();

	if (LookAxisVector.X != 0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if (LookAxisVector.Y != 0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARPGPlayerCharacter::InitializeCharacterConfig()
{
	if (!CharacterConfig)
	{
		UE_LOG(LogRPGPlayerCharacter, Warning,
		       TEXT("ARPGPlayerCharacter::InitializeCharacterConfig - CharacterConfig is null!"));
		return;
	}

	// 装备默认武器
	//EquipWeapon(CharacterConfig->DefaultWeaponType);
}
void ARPGPlayerCharacter::EquipWeapon(ERPGWeaponType NewWeaponType)
{
	// 如果已经装备了相同武器，跳过
	if (CurrentWeaponType == NewWeaponType && NewWeaponType != ERPGWeaponType::None)
	{
		return;
	}

	// 更新当前武器类型
	ERPGWeaponType OldWeaponType = CurrentWeaponType;
	CurrentWeaponType = NewWeaponType;

	// 切换动画层并应用武器战斗数据
	if (CachedAnimInstance)
	{
		TSubclassOf<UAnimInstance> NewAnimLayerClass = nullptr;
		
		// 黑魂模式：从武器数据中获取动画层
		if (UPlayerCombatComponent* CombatComp = FindComponentByClass<UPlayerCombatComponent>())
		{
			if (ARPGPlayerWeapon* Weapon = CombatComp->GetPlayerCurrentEquippedWeapon())
			{
				NewAnimLayerClass = Weapon->PlayerWeaponData.WeaponAnimLayerToLink;
				// 应用武器战斗参数（连招、攻速等）
				CombatComp->ApplyWeaponData(Weapon->PlayerWeaponData);
			}
		}

		if (NewAnimLayerClass)
		{
			CachedAnimInstance->LinkAnimLayer(NewAnimLayerClass);
			UE_LOG(LogRPGPlayerCharacter, Log,
			       TEXT("ARPGPlayerCharacter::EquipWeapon - Switched to weapon: %d, AnimLayer: %s"),
			       static_cast<int32>(NewWeaponType), *NewAnimLayerClass->GetName());
		}
		else if (NewWeaponType == ERPGWeaponType::None)
		{
			// 变换武器时取消链接
			CachedAnimInstance->UnlinkAnimLayer();
			UE_LOG(LogRPGPlayerCharacter, Log, TEXT("ARPGPlayerCharacter::EquipWeapon - Unlinked anim layer"));
		}
	}

	// TODO: 在这里添加武器网格体切换逻辑
	// TODO: 在这里添加 GAS 属性更新逻辑
}

UPlayerCombatComponent* ARPGPlayerCharacter::GetPlayerCombatComponent() const
{
	return PlayerCombatComponent;
}

void ARPGPlayerCharacter::Input_AbilityInputPressed(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Log, TEXT("Input_AbilityInputPressed: InputTag [%s]"), *InputTag.ToString());
	if (ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>())
	{
		if (URPGAbilitySystemComponent* ASC = PS->GetRPGAbilitySystemComponent())
		{
			ASC->OnAbilityInputPressed(InputTag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Input_AbilityInputPressed: ASC is null!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Input_AbilityInputPressed: PlayerState is null!"));
	}
}

void ARPGPlayerCharacter::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>())
	{
		if (URPGAbilitySystemComponent* ASC = PS->GetRPGAbilitySystemComponent())
		{
			ASC->OnAbilityInputReleased(InputTag);
		}
	}
}
