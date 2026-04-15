// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Types/RPGEnumTypes.h"
#include "RPGPlayerCharacter.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class UDataAsset_InputConfig;
class UDataAsset_CharacterConfig;
class USpringArmComponent;
class UCameraComponent;
class URPGCharacterAnimInstance;
class UPlayerCombatComponent;
class URPGAbilitySystemComponent;
class URPGAttributeSet;
/**
 * 
 */
UCLASS()
class RPG_API ARPGPlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ARPGPlayerCharacter();

	// IAbilitySystemInterface - 从 PlayerState 获取 ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 服务端：Controller Possess 时初始化 ASC ActorInfo
	virtual void PossessedBy(AController* NewController) override;

	// 客户端：PlayerState 复制到位后初始化 ASC ActorInfo
	virtual void OnRep_PlayerState() override;

	// 初始化 ASC ActorInfo（服务端/客户端共用）
	void InitAbilityActorInfo();

public:
	// ========== 装备系统 ==========
	
	/** 装备武器并切换动画层 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipWeapon(ERPGWeaponType NewWeaponType);

	/** 获取当前武器类型 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	ERPGWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

	/** 获取角色配置 */
	UFUNCTION(BlueprintPure, Category = "Character")
	UDataAsset_CharacterConfig* GetCharacterConfig() const { return CharacterConfig; }

	/** 获取玩家战斗组件 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	UPlayerCombatComponent* GetPlayerCombatComponent() const;

private:
	//相机
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// 输入配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData", meta=(AllowPrivateAccess = "true"))
	UDataAsset_InputConfig* InputConfigDataAsset;

	// 角色配置数据资产
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_CharacterConfig> CharacterConfig;

	// ========== 转向控制 ==========
	
	/** 基础转向速度（度/秒）- 静止或低速时的转向速率 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation", meta=(AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1000.0"))
	float BaseTurnSpeed;

	/** 最大转向速度（度/秒）- 高速移动时的转向速率上限 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation", meta=(AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1500.0"))
	float MaxTurnSpeed;

	/** 速度影响系数 - 行进速度对转向的影响程度 (0=无影响, 1=完全匹配) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation", meta=(AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float SpeedTurnMultiplier;

	/** 角度影响系数 - 转体角度对转向速度的影响程度 (0=无影响, 1=线性增加) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation", meta=(AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float AngleTurnMultiplier;

	/** 目标朝向旋转 */
	FRotator TargetRotation;

	/** 是否有有效的输入方向 */
	bool bHasMovementInput;

	// ========== 调试显示 ==========
	
	/** 是否显示转向调试射线（绿色=当前朝向，红色=目标朝向） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta=(AllowPrivateAccess = "true"))
	bool bShowRotationDebug;

	// 定时器句柄
	FTimerHandle DebugTimerHandle;

	// 定时器回调
	UFUNCTION()
	void OnDebugTimerTick();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	UPlayerCombatComponent* PlayerCombatComponent;

	// 当前武器类型
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta=(AllowPrivateAccess = "true"))
	ERPGWeaponType CurrentWeaponType;

	// 动画实例缓存
	UPROPERTY()
	TObjectPtr<URPGCharacterAnimInstance> CachedAnimInstance;

	/** 初始化角色配置 */
	void InitializeCharacterConfig();

	/** 平滑转向到目标朝向 */
	void SmoothRotateToTarget(float DeltaTime);

	/** 计算动态转向速度（基于行进速度和转向角度） */
	float CalculateDynamicTurnSpeed() const;

	/** 绘制转向调试射线 */
	void DrawRotationDebug();

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);
};
