# 玩家攻击→敌人伤害→敌人死亡 系统文档

**创建日期**: 2026-04-26  
**状态**: C++部分完成 ✅ | 蓝图配置待完成 ⏳  
**参考项目**: Warrior项目GAS架构

---

## 一、系统概述

### 1.1 功能描述

实现完整的近战伤害处理流程:
- 玩家发动攻击 → 武器碰撞检测 → 发送GAS事件 → 伤害计算 → 敌人扣血 → 死亡判定 → 敌人销毁

### 1.2 设计原则

- **Warrior标准**: 采用Warrior项目的GAS事件驱动架构
- **职责分离**: 碰撞检测、事件分发、伤害计算、结果处理解耦
- **数据驱动**: 通过SetByCaller动态传递伤害参数
- **最小传递**: EventData只携带Instigator和Target,蓝图GA自行查询详细数据

---

## 二、系统架构

### 2.1 核心组件关系图

```
┌─────────────────────────────────────────────────────────────────────┐
│                        玩家攻击系统                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌───────────────────────┐                                          │
│  │URPGPlayerAbility_     │                                          │
│  │AttackCombo            │                                          │
│  │                       │                                          │
│  │• ActivateAbility()    │                                          │
│  │• HandleApplyDamage()  │                                          │
│  │• SendHitReactEvent()  │                                          │
│  └──────────┬────────────┘                                          │
│             │                                                       │
│             │ 1. 监听Shared_Event_MeleeHit                          │
│             ▼                                                       │
│  ┌───────────────────────┐         ┌──────────────────────┐        │
│  │WaitMeleeHitEventTask  │◄────────│SendGameplayEventTo   │        │
│  │(UAbilityTask)         │         │Actor                 │        │
│  └───────────────────────┘         └──────────┬───────────┘        │
│                                                │                   │
│  ┌───────────────────────┐                    │ 2. 触发事件        │
│  │UPlayerCombatComponent │◄───────────────────┘                   │
│  │                       │                                        │
│  │• OnHitTargetActor()   │         ┌──────────────────────┐       │
│  │• CurrentComboType     │         │AN_SendGameplayEvent  │       │
│  │• ComboCounts          │◄────────│ToOwner               │       │
│  │• SetCurrentComboType()│         │(动画通知)             │       │
│  └───────────────────────┘         └──────────────────────┘       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              │ 3. 应用GE
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        伤害计算系统                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌───────────────────────┐         ┌──────────────────────┐        │
│  │MakePlayerDamage       │         │UGEExecCale_          │        │
│  │EffectSpecHandle()     │────────►│DamageTaken           │        │
│  │                       │         │                      │        │
│  │输入:                   │         │计算逻辑:              │        │
│  │• EffectClass          │         │• 捕获攻击/防御属性   │        │
│  │• BaseDamage           │         │• 读取SetByCaller     │        │
│  │• AttackType Tag       │         │• 连招加成计算        │        │
│  │• ComboCount           │         │• FinalDamage输出     │        │
│  └───────────────────────┘         └──────────┬───────────┘        │
│                                                │                   │
│                                                ▼                   │
│                               ┌──────────────────────────────┐    │
│                               │URPGAttributeSet              │    │
│                               │                              │    │
│                               │• PostGameplayEffectExecute() │    │
│                               │                              │    │
│                               │处理:                          │    │
│                               │• DamageTaken → Health扣减   │    │
│                               │• 死亡检测 (Health <= 0)     │    │
│                               └──────────┬───────────────────┘    │
│                                          │                        │
└──────────────────────────────────────────┼────────────────────────┘
                                           │ 4. 调用Die()
                                           ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        敌人死亡系统                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────────────────────────────────────────────────┐      │
│  │ARPGEnemyCharacter::Die()                                 │      │
│  │                                                          │      │
│  │  Step 1: 设置 Shared_Status_Dead Tag                    │      │
│  │  Step 2: 更新Blackboard (Dead = true)                   │      │
│  │  Step 3: 禁用EnemyCombatComponent                       │      │
│  │  Step 4: 禁用碰撞盒                                     │      │
│  │  Step 5: 停止移动                                       │      │
│  │  Step 6: 播放死亡动画 (通过AnimBlueprint检测Tag)        │      │
│  │  Step 7: SetLifeSpan(5.0f) 自动销毁                     │      │
│  └──────────────────────────────────────────────────────────┘      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 组件清单

| 组件 | 类型 | 职责 | 文件路径 |
|------|------|------|---------|
| `URPGPlayerAbility_AttackCombo` | C++ GA | 攻击连招主逻辑,监听MeleeHit事件,处理伤害应用 | `Source/RPG/Public/AbilitySystem/Abilities/Player/RPGPlayerAbility_AttackCombo.h` |
| `UPlayerCombatComponent` | C++ Component | 战斗状态管理,碰撞检测,连招计数 | `Source/RPG/Public/Component/Combat/PlayerCombatComponent.h` |
| `UGEExecCale_DamageTaken` | C++ ExecCalc | 伤害数值计算(攻击/防御比例,连招加成) | `Source/RPG/Private/AbilitySystem/Abilities/GEExecCale/GEExecCale_DamageTaken.cpp` |
| `URPGAttributeSet` | C++ AttributeSet | 属性管理,伤害应用,死亡检测 | `Source/RPG/Public/AbilitySystem/RPGAttributeSet.h` |
| `ARPGEnemyCharacter` | C++ Character | 敌人角色基类,死亡流程实现 | `Source/RPG/Public/Character/RPGEnemyCharacter.h` |
| `BP_GE_Share_DealDamage` | 蓝图GE | 伤害效果配置(待创建) | `Content/RPG/...` |

---

## 三、详细执行流程

### 3.1 完整时序图

```
Player Character          Attack GA              Combat Component          Enemy Character          Enemy ASC
      │                      │                        │                        │                        │
      │  1. 输入攻击          │                        │                        │                        │
      ├─────────────────────►│                        │                        │                        │
      │                      │                        │                        │                        │
      │  2. ActivateAbility  │                        │                        │                        │
      │                      │                        │                        │                        │
      │                      │─┐                      │                        │                        │
      │                      ││ 3. SetCurrentComboType│                        │                        │
      │                      ├─┼─────────────────────►│                        │                        │
      │                      ││                      │                        │                        │
      │                      ││ 4. WaitMeleeHitEvent  │                        │                        │
      │                      ││                      │                        │                        │
      │                      ││ 5. PlayMontage        │                        │                        │
      │                      │┘                      │                        │                        │
      │                      │                        │                        │                        │
      │                      │     6. 动画播放中...   │                        │                        │
      │                      │                        │                        │                        │
      │                      │     7. AN触发碰撞盒    │                        │                        │
      │                      ├───────────────────────►│                        │                        │
      │                      │                        │                        │                        │
      │                      │     8. OnHitTargetActor│                        │                        │
      │                      │                        │                        │                        │
      │                      │     9. SendGameplayEventToActor(自己)           │                        │
      │                      │◄───────────────────────┤                        │                        │
      │                      │                        │                        │                        │
      │                      │  10. EventReceived     │                        │                        │
      │                      │                        │                        │                        │
      │                      │  11. HandleApplyDamage │                        │                        │
      │                      │     ┌──────────────┐   │                        │                        │
      │                      │     │• 获取Target  │   │                        │                        │
      │                      │     │• 获取Damage  │   │                        │                        │
      │                      │     │• 创建Spec    │   │                        │                        │
      │                      │     └──────────────┘   │                        │                        │
      │                      │                        │                        │                        │
      │                      │  12. NativeApplyEffectSpecHandleToTarget       │                        │
      │                      ├───────────────────────────────────────────────►│                        │
      │                      │                        │                        │                        │
      │                      │                        │     13. Execute Calc  │                        │
      │                      │                        │     14. PostExecute   │                        │
      │                      │                        │                        │                        │
      │                      │                        │     15. 检测死亡      │                        │
      │                      │                        ├───────────────────────►│                        │
      │                      │                        │                        │                        │
      │                      │                        │     16. Die()         │                        │
      │                      │                        │◄───────────────────────┤                        │
      │                      │                        │                        │                        │
      │                      │  17. SendHitReactEvent │                        │                        │
      │                      ├───────────────────────────────────────────────►│                        │
      │                      │                        │                        │                        │
```

### 3.2 流程详解

#### 阶段1: 攻击激活 (ActivateAbility)

**文件**: `RPGPlayerAbility_AttackCombo.cpp`  
**函数**: `ActivateAbility()`

```cpp
// 1. 从CombatComponent同步连招计数
CombatComp->SwitchComboType(ComboType);
CurrentLightAttackComboCount = CombatComp->GetComboCount(ComboType);

// 2. 设置当前攻击类型 (用于伤害计算)
CombatComp->SetCurrentComboType(ComboType);

// 3. 创建事件监听Task
WaitMeleeHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this,
    RPGGameplayTags::Shared_Event_MeleeHit,
    nullptr, false, true
);
WaitMeleeHitEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::HandleApplyDamage);
WaitMeleeHitEventTask->ReadyForActivation();

// 4. 播放攻击蒙太奇
PlayCurrentComboMontage();
```

**关键点**:
- ✅ 攻击GA在激活时立即设置事件监听
- ✅ EventReceived委托绑定到HandleApplyDamage
- ✅ 设置CurrentComboType供后续伤害计算使用

---

#### 阶段2: 碰撞检测与事件发送

**文件**: `PlayerCombatComponent.cpp`  
**函数**: `OnHitTargetActor()`

```cpp
void UPlayerCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    if(OverlappedActors.Contains(HitActor)) return;
    OverlappedActors.AddUnique(HitActor);

    // 最小传递: 只传Instigator和Target
    FGameplayEventData Data;
    Data.Instigator = GetOwningPawn();
    Data.Target = HitActor;

    // 发送到攻击者自己的ASC (Warrior标准)
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetOwningPawn(),
        RPGGameplayTags::Shared_Event_MeleeHit,
        Data
    );

    // 发送HitPause事件 (打击感)
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetOwningPawn(),
        RPGGameplayTags::Player_Event_HitPause,
        FGameplayEventData()
    );
}
```

**关键点**:
- ✅ 不传递EventMagnitude,由GA自行查询CombatComponent
- ✅ 事件发送给攻击者自己 (不是目标)
- ✅ 符合GAS标准实践

---

#### 阶段3: 伤害处理 (HandleApplyDamage)

**文件**: `RPGPlayerAbility_AttackCombo.cpp`  
**函数**: `HandleApplyDamage()`

```cpp
void URPGPlayerAbility_AttackCombo::HandleApplyDamage(FGameplayEventData EventData)
{
    // 1. 获取目标
    AActor* TargetActor = const_cast<AActor*>(EventData.Target.Get());
    
    // 2. 获取伤害数据
    float PlayerLevel = OwningASC->GetDefaultAttributeValue(URPGAttributeSet::GetPlayerLevelAttribute());
    float BaseDamage = CombatComp->GetPlayerCurrentEquippedWeaponDamageAtLevel(PlayerLevel);
    int32 ComboCount = CombatComp->GetComboCount(ComboType);
    
    // 3. 确定攻击类型Tag
    FGameplayTag AttackTypeTag;
    if (ComboType == ERPGComboType::LightAttack)
        AttackTypeTag = RPGGameplayTags::Player_SetByCaller_AttackType_Light;
    else if (ComboType == ERPGComboType::HeavyAttack)
        AttackTypeTag = RPGGameplayTags::Player_SetByCaller_AttackType_Heavy;
    
    // 4. 创建GE Spec (使用基类辅助函数)
    FGameplayEffectSpecHandle SpecHandle = MakePlayerDamageEffectSpecHandle(
        DamageEffectClass,
        BaseDamage,
        AttackTypeTag,
        ComboCount
    );
    
    // 5. 应用到目标 (使用基类辅助函数)
    NativeApplyEffectSpecHandleToTarget(TargetActor, SpecHandle);
    
    // 6. 发送HitReact事件
    SendHitReactEvent(TargetActor);
}
```

**关键点**:
- ✅ 使用基类辅助函数 `MakePlayerDamageEffectSpecHandle` 和 `NativeApplyEffectSpecHandleToTarget`
- ✅ SetByCaller传递: BaseDamage, ComboCount, AttackType
- ✅ 自动设置ContextHandle (Ability, SourceObject, Instigator)

---

#### 阶段4: 伤害计算 (ExecCalc)

**文件**: `GEExecCale_DamageTaken.cpp`  
**类**: `UGEExecCale_DamageTaken`

```cpp
void UGEExecCale_DamageTaken::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    // 1. 捕获属性
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    
    float SourceAttackPower = 0.f;
    float TargetDefensePower = 0.f;
    float TargetDamageTaken = 0.f;
    
    // 2. 读取SetByCaller
    float BaseDamage = Spec.GetSetByCallerMagnitude(RPGGameplayTags::Shared_SetByCaller_BaseDamage);
    float LightComboCount = Spec.GetSetByCallerMagnitude(RPGGameplayTags::Player_SetByCaller_AttackType_Light);
    float HeavyComboCount = Spec.GetSetByCallerMagnitude(RPGGameplayTags::Player_SetByCaller_AttackType_Heavy);
    
    // 3. 计算连招加成
    float ComboMultiplier = 1.0f;
    if (LightComboCount > 0)
        ComboMultiplier = (LightComboCount - 1) * 0.05f + 1.0f;
    else if (HeavyComboCount > 0)
        ComboMultiplier = HeavyComboCount * 0.15f + 1.0f;
    
    // 4. 最终伤害公式
    float FinalDamage = BaseDamage * ComboMultiplier * SourceAttackPower / TargetDefensePower;
    
    // 5. 输出到DamageTaken
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            URPGAttributeSet::GetDamageTakenAttribute(),
            EGameplayModOp::Override,
            FinalDamage
        )
    );
}
```

**计算公式**:
```
FinalDamage = BaseDamage × ComboMultiplier × (AttackPower / DefensePower)

连招加成:
- 轻攻击: (ComboCount - 1) × 0.05 + 1.0
  - 1连: 1.0x
  - 2连: 1.05x
  - 3连: 1.10x
  
- 重攻击: ComboCount × 0.15 + 1.0
  - 1连: 1.15x
  - 2连: 1.30x
  - 3连: 1.45x
```

---

#### 阶段5: 属性更新与死亡检测

**文件**: `RPGAttributeSet.cpp`  
**函数**: `PostGameplayEffectExecute()`

```cpp
void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
    {
        const float OldHealth = GetCurrentHealth();
        const float DamageDone = GetDamageTaken();
        const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.0f, GetMaxHealth());
        
        SetCurrentHealth(NewCurrentHealth);
        SetDamageTaken(0.0f);

        // ⏳ TODO: 死亡检测
        if (NewCurrentHealth <= 0.0f)
        {
            // 需要调用ARPGEnemyCharacter::Die()
            // 通过事件或接口调用,保持解耦
        }
    }
}
```

---

#### 阶段6: 敌人死亡流程

**文件**: `RPGEnemyCharacter.cpp`  
**函数**: `Die()`

```cpp
void ARPGEnemyCharacter::Die()
{
    UE_LOG(LogRPGEnemyCharacter, Warning, TEXT("[Enemy] Die() called on %s"), *GetName());

    // 1. 设置死亡Tag
    if (RPGAbilitySystemComponent)
    {
        RPGAbilitySystemComponent->AddLooseGameplayTag(RPGGameplayTags::Shared_Status_Dead);
    }

    // 2. 通知AI控制器更新Blackboard
    if (CachedAIController.IsValid())
    {
        UBlackboardComponent* Blackboard = CachedAIController->GetBlackboardComponent();
        if (Blackboard)
        {
            Blackboard->SetValueAsBool(FName("Dead"), true);
        }
    }

    // 3. 禁用战斗组件
    if (EnemyCombatComponent)
    {
        EnemyCombatComponent->SetComponentTickEnabled(false);
    }

    // 4. 禁用碰撞盒
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    // 5. 停止移动
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->SetMovementMode(MOVE_None);
    }

    // 6. 播放死亡动画 (通过AnimBlueprint检测Tag)
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    
    // 7. 设置自动销毁时间
    SetLifeSpan(5.0f);
}
```

**已验证**: 7步全部执行成功 ✅

---

## 四、关键技术决策

### 4.1 EventData传递策略

| 方案 | 传递内容 | 选择 | 原因 |
|------|---------|------|------|
| 方案A (最小传递) | Instigator + Target | ✅ 采用 | 符合GAS标准,Warrior实践 |
| 方案B (完整传递) | 所有伤害数据 | ❌ 拒绝 | 冗余,不易扩展 |
| 方案C (OptionalObject) | 自定义UObject | ❌ 拒绝 | 需要额外类,复杂度高 |

### 4.2 伤害处理职责归属

| 方案 | 职责位置 | 选择 | 原因 |
|------|---------|------|------|
| 方案A (被动GA) | 单独GA监听事件 | ❌ 拒绝 | Warrior用蓝图所以需要中转,C++无此限制 |
| 方案B (集成到攻击GA) | URPGPlayerAbility_AttackCombo | ✅ 采用 | 数据就近,无事件开销,代码简洁 |

### 4.3 事件发送目标

| 方案 | 发送目标 | 选择 | 原因 |
|------|---------|------|------|
| 方案A (发送给自己) | 攻击者ASC | ✅ 采用 | Warrior标准,被动GA在攻击者身上 |
| 方案B (发送给目标) | 目标ASC | ❌ 拒绝 | 不符合GAS标准实践 |

---

## 五、已解决问题

### 5.1 ValidData vs EventReceived

**问题**: 使用 `WaitMeleeHitEventTask->ValidData.AddDynamic()` 编译失败  
**原因**: `UAbilityTask_WaitGameplayEvent` 的事件委托名是 `EventReceived`,不是 `ValidData`  
**解决**: 改为 `WaitMeleeHitEventTask->EventReceived.AddDynamic()`  
**参考**: 对比 `RPGPlayerAbility_EquipSword.cpp` 第243行

### 5.2 函数签名不匹配

**问题**: `HandleApplyDamage(const FGameplayEventData& EventData)` 委托绑定失败  
**原因**: `EventReceived` 委托按值传递 `FGameplayEventData`,不是const引用  
**解决**: 改为 `HandleApplyDamage(FGameplayEventData EventData)`  
**参考**: 对比 `RPGPlayerAbility_Jump.cpp` 第176行

### 5.3 SetHiddenInGame编译错误

**问题**: `UActorComponent` 调用 `SetHiddenInGame()` 编译失败  
**原因**: `SetHiddenInGame` 是 `AActor` 的方法,不是 `UActorComponent` 的方法  
**解决**: 移除该行 (组件隐藏由Actor控制)

### 5.4 CharacterMovement变量名冲突

**问题**: 局部变量 `CharacterMovement` 声明隐藏了 `ACharacter` 的成员变量  
**原因**: 变量名与父类成员同名 (C4458警告)  
**解决**: 重命名为 `MovementComponent`

### 5.5 UBlackboardComponent不完整类型

**问题**: 使用 `UBlackboardComponent` 编译失败 "incomplete type"  
**原因**: 缺少头文件引用  
**解决**: 添加 `#include "BehaviorTree/BlackboardComponent.h"`

---

## 六、待完成任务

### 6.1 蓝图GE配置 (⏳ 优先级: 高)

**任务**: 创建 `BP_GE_Share_DealDamage`

**步骤**:
1. 在Content Browser创建 GameplayEffect 蓝图: `Content/RPG/AbilitySystem/Effect/BP_GE_Share_DealDamage`
2. 配置 Execution:
   - Add Execution → 选择 `GEExecCale_DamageTaken`
   - Source: Player (攻击者)
   - Target: Enemy (目标)
3. 配置 SetByCaller Magnitudes:
   - Add Magnitude → `Shared.SetByCaller.BaseDamage` (Float)
   - Add Magnitude → `Player.SetByCaller.AttackType.Light` (Float)
   - Add Magnitude → `Player.SetByCaller.AttackType.Heavy` (Float)

### 6.2 攻击GA蓝图配置 (⏳ 优先级: 高)

**任务**: 在攻击GA蓝图子类中设置DamageEffectClass

**步骤**:
1. 打开 `BP_Ability_LightAttack1` (或对应的蓝图子类)
2. 在Class Defaults中找到 `DamageEffectClass`
3. 设置为 `BP_GE_Share_DealDamage`

### 6.3 死亡检测调用 (⏳ 优先级: 中)

**任务**: 在 `PostGameplayEffectExecute` 中调用 `Die()`

**方案A (推荐)**: 通过GameplayEvent
```cpp
if (NewCurrentHealth <= 0.0f)
{
    // 发送死亡事件
    FGameplayEventData DeathData;
    DeathData.Instigator = Data.EffectSpec.GetContext().GetOriginalSource();
    DeathData.Target = Data.Target.GetAvatarActor();
    
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        Data.Target.GetAvatarActor(),
        RPGGameplayTags::Shared_Event_Death,
        DeathData
    );
}
```

**方案B**: 直接调用 (需要Cast)
```cpp
if (NewCurrentHealth <= 0.0f)
{
    if (ARPGEnemyCharacter* Enemy = Cast<ARPGEnemyCharacter>(Data.Target.GetAvatarActor()))
    {
        Enemy->Die();
    }
}
```

### 6.4 能力注册验证 (⏳ 优先级: 低)

**任务**: 确认攻击GA已注册到ASC

**检查点**:
- 在 `RPGPlayerCharacter` 或 `RPGAbilitySystemComponent` 的StartupData中
- 确认 `URPGPlayerAbility_AttackCombo` 的子类已添加

---

## 七、测试清单

### 7.1 单元测试

| 测试项 | 预期结果 | 状态 |
|--------|---------|------|
| 武器碰撞检测 | OnHitTargetActor触发 | ⏳ |
| MeleeHit事件发送 | 攻击GA收到事件 | ⏳ |
| HandleApplyDamage执行 | 日志输出完整 | ⏳ |
| SetByCaller传递 | BaseDamage, ComboCount正确 | ⏳ |
| GE应用到敌人 | ExecCalc执行 | ⏳ |
| 伤害计算 | FinalDamage符合公式 | ⏳ |
| Health扣减 | CurrentHealth正确更新 | ⏳ |
| 死亡判定 | Die()调用 | ✅ 已验证 |
| 敌人销毁 | SetLifeSpan生效 | ✅ 已验证 |

### 7.2 集成测试

| 测试场景 | 预期行为 | 状态 |
|---------|---------|------|
| 轻攻击1连击 | BaseDamage × 1.0 × 攻击/防御 | ⏳ |
| 轻攻击3连击 | BaseDamage × 1.10 × 攻击/防御 | ⏳ |
| 重攻击2连击 | BaseDamage × 1.30 × 攻击/防御 | ⏳ |
| 连续攻击不同敌人 | 每个敌人独立受伤 | ⏳ |
| 敌人死亡后销毁 | 5秒后自动销毁 | ✅ 已验证 |

---

## 八、文件清单

### 8.1 修改的文件

| 文件 | 修改内容 | 行数变化 |
|------|---------|---------|
| `RPGPlayerAbility_AttackCombo.h` | 新增伤害处理函数声明,任务引用,GE类配置 | +17 |
| `RPGPlayerAbility_AttackCombo.cpp` | 实现HandleApplyDamage, SendHitReactEvent, 事件监听 | +111 |
| `PlayerCombatComponent.h` | 新增CurrentComboType状态管理 | +11 |
| `PlayerCombatComponent.cpp` | 实现SetCurrentComboType | +6 |
| `RPGEnemyCharacter.h` | 新增Die()声明 | +3 |
| `RPGEnemyCharacter.cpp` | 实现Die()函数,添加必要头文件 | +45 |

### 8.2 依赖的文件

| 文件 | 用途 |
|------|------|
| `RPGPlayerGameplayAbility.cpp` | MakePlayerDamageEffectSpecHandle辅助函数 |
| `RPGGameplayAbility.cpp` | NativeApplyEffectSpecHandleToTarget辅助函数 |
| `GEExecCale_DamageTaken.cpp` | 伤害计算逻辑 (既存) |
| `RPGAttributeSet.cpp` | 属性管理 (既存) |
| `RPGGameplayTags.h/cpp` | GameplayTag定义 (既存) |

---

## 九、参考资料

### 9.1 Warrior项目对应实现

| Warrior组件 | 本项目对应 | 备注 |
|------------|-----------|------|
| `GA_Hero_LightAttackMaster` | `URPGPlayerAbility_AttackCombo` | Warrior用蓝图,我们用C++ |
| `HandleApplyDamage` (蓝图事件) | `HandleApplyDamage` (C++函数) | 逻辑一致 |
| `MakeHeroDamageEffectSpecHandle` | `MakePlayerDamageEffectSpecHandle` | 基类辅助函数 |
| `UHeroCombatComponent` | `UPlayerCombatComponent` | 职责相同 |

### 9.2 GAS标准实践

- ✅ EventData只传递最小必要信息
- ✅ 事件发送给攻击者自己 (不是目标)
- ✅ 使用SetByCaller动态传递伤害参数
- ✅ ExecCalc处理复杂计算,PostExecute处理属性更新
- ✅ 通过Tag和事件解耦组件

---

## 十、总结

### 10.1 完成度

| 模块 | 进度 | 备注 |
|------|------|------|
| C++代码实现 | 100% ✅ | 全部编译通过 |
| 蓝图配置 | 0% ⏳ | 需在编辑器操作 |
| 死亡检测调用 | 50% ⏳ | Die()已实现,待调用 |
| 测试验证 | 20% ⏳ | 仅验证Die() |

### 10.2 下一步行动

1. **立即**: 创建BP_GE_Share_DealDamage并配置
2. **立即**: 在攻击GA蓝图中设置DamageEffectClass
3. **优先**: 实现PostGameplayEffectExecute中的死亡检测调用
4. **后续**: 完整测试伤害流程
5. **后续**: 优化日志输出 (生产环境可关闭部分日志)

---

**文档版本**: v1.0  
**最后更新**: 2026-04-26  
**维护者**: AI Assistant
