# 连招系统定时器问题诊断报告

## 📋 问题概述

**问题描述**：PlayerAttackCombo 连招系统的 Combo Window 定时器（3秒）无法触发重置逻辑，导致连招计数不会在超时后重置。

**影响范围**：URPGPlayerAbility_AttackCombo 连招系统

**发现时间**：2026-04-15

---

## 🔍 问题现象

### 日志表现

```
Wed Apr 15 13:55:15 CST 2026  Log  [PlayerAttackCombo] Combo window timer started via K2_SetTimerDelegate (3.00 seconds)
Wed Apr 15 13:55:15 CST 2026  Log  [PlayerAttackCombo] Timer is active: YES
Wed Apr 15 13:55:15 CST 2026  Log  [PlayerAttackCombo] Timer handle valid: YES
# 3秒后没有任何 ResetComboCount 相关日志
```

### 行为表现

1. ✅ 定时器成功创建并激活
2. ✅ Timer Handle 有效
3. ❌ **3秒后定时器回调未触发**
4. ❌ 连招计数持续递增，不会重置

---

## 🐛 根本原因分析

### 原因一：使用蓝图函数设置定时器（首次问题）

**问题代码**：
```cpp
// ❌ 错误：使用 K2_SetTimerDelegate（蓝图函数）
ComboCountResetTimerHandle = UKismetSystemLibrary::K2_SetTimerDelegate(
    TimerDelegate,
    ComboWindowTime,
    false
);
```

**问题分析**：
- `K2_SetTimerDelegate` 是为蓝图设计的 Kismet 函数
- 在 C++ Gameplay Ability 中使用时，委托绑定可能无法正确工作
- 定时器虽然被创建，但回调机制存在兼容性问题

**解决方案**：
```cpp
// ✅ 正确：使用 C++ 原生 TimerManager
World->GetTimerManager().SetTimer(
    ComboCountResetTimerHandle,
    TimerDelegate,
    ComboWindowTime,
    false
);
```

---

### 原因二：定时器绑定到 Ability 实例（核心问题）

**问题代码**：
```cpp
// ❌ 错误：绑定到 Ability 实例（this）
FTimerDynamicDelegate TimerDelegate;
TimerDelegate.BindUFunction(this, FName("ResetComboCount"));

World->GetTimerManager().SetTimer(
    ComboCountResetTimerHandle,
    TimerDelegate,  // 绑定到 Ability 实例
    ComboWindowTime,
    false
);
```

**问题分析**：

1. **Ability 生命周期管理**：
   - `InstancedPerActor` 策略下，同一个 Actor 对同一 Ability 只创建一个实例
   - `EndAbility` 后，Ability 实例并未销毁，而是回收到池中
   - 但 Ability 实例的状态可能变为非活跃状态

2. **委托失效**：
   - 定时器绑定到 Ability 实例的 `ResetComboCount` 函数
   - 当 Ability 结束后，UObject 可能不再正确接收定时器回调
   - 即使实例还在内存中，回调机制也可能失效

3. **架构设计问题**：
   - 连招状态（CurrentLightAttackComboCount）存储在 Ability 中
   - 定时器也设置在 Ability 上
   - 这违反了"状态应该在持久化组件中管理"的设计原则

**解决方案**：将连招状态和定时器管理迁移到 PlayerCombatComponent

---

### 原因三：定时器回调函数缺少 UFUNCTION 标记

**问题代码**：
```cpp
// ❌ 错误：缺少 UFUNCTION() 标记
private:
    void OnComboWindowTimerExpired();
```

**问题分析**：
- `BindUFunction` 通过 UE 反射系统查找函数
- 没有 `UFUNCTION()` 标记的函数不会被反射系统注册
- 导致委托绑定失败，定时器无法触发回调

**解决方案**：
```cpp
// ✅ 正确：添加 UFUNCTION() 标记
private:
    UFUNCTION()
    void OnComboWindowTimerExpired();
```

---

## ✅ 最终解决方案

### 架构调整：状态与逻辑分离

**设计原则**：
- **PlayerCombatComponent**：管理连招状态（计数、定时器）
- **URPGPlayerAbility_AttackCombo**：管理连招逻辑（播放蒙太奇、输入处理）

---

### 修改文件清单

#### 1. PlayerCombatComponent.h

**新增内容**：
```cpp
public:
    UPlayerCombatComponent();
    
    // 连招状态管理
    UFUNCTION(BlueprintCallable, Category="RPG|Combo")
    int32 GetCurrentComboCount() const { return CurrentComboCount; }

    UFUNCTION(BlueprintCallable, Category="RPG|Combo")
    void SetCurrentComboCount(int32 NewCount) { CurrentComboCount = NewCount; }

    UFUNCTION(BlueprintCallable, Category="RPG|Combo")
    void ResetComboCount();

    UFUNCTION(BlueprintCallable, Category="RPG|Combo")
    void AdvanceComboCount(int32 MaxComboCount);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    // 连招状态
    int32 CurrentComboCount = 1;

    // 连招窗口定时器
    FTimerHandle ComboResetTimerHandle;

    // 定时器回调（必须有 UFUNCTION 标记）
    UFUNCTION()
    void OnComboWindowTimerExpired();
```

#### 2. PlayerCombatComponent.cpp

**实现逻辑**：
```cpp
UPlayerCombatComponent::UPlayerCombatComponent()
{
    CurrentComboCount = 1;
}

void UPlayerCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentComboCount = 1;
}

void UPlayerCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void UPlayerCombatComponent::ResetComboCount()
{
    CurrentComboCount = 1;
    UE_LOG(LogRPGPlayerCombatComponent, Log, 
        TEXT("[PlayerCombatComponent] Combo count reset to 1 (timer expired)"));
}

void UPlayerCombatComponent::AdvanceComboCount(int32 MaxComboCount)
{
    if (CurrentComboCount >= MaxComboCount)
    {
        CurrentComboCount = 1;
        UE_LOG(LogRPGPlayerCombatComponent, Log, 
            TEXT("[PlayerCombatComponent] Max combo reached, resetting to 1"));
    }
    else
    {
        CurrentComboCount++;
        UE_LOG(LogRPGPlayerCombatComponent, Log, 
            TEXT("[PlayerCombatComponent] Combo count advanced to %d"), CurrentComboCount);
    }
}

void UPlayerCombatComponent::OnComboWindowTimerExpired()
{
    UE_LOG(LogRPGPlayerCombatComponent, Warning, 
        TEXT("[PlayerCombatComponent] >>> Combo window timer expired, resetting combo <<<"));
    ResetComboCount();
}
```

#### 3. RPGPlayerAbility_AttackCombo.cpp

**修改 ActivateAbility**：
```cpp
void URPGPlayerAbility_AttackCombo::ActivateAbility(...)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // ✅ 从 CombatComponent 同步连招计数
    if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
    {
        CurrentLightAttackComboCount = CombatComp->GetCurrentComboCount();
        UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, 
            TEXT("[PlayerAttackCombo] Synced combo count from CombatComponent: %d"), 
            CurrentLightAttackComboCount);
    }
    
    PlayCurrentComboMontage();
}
```

**修改 AdvanceComboCount**：
```cpp
void URPGPlayerAbility_AttackCombo::AdvanceComboCount()
{
    // ✅ 使用 CombatComponent 管理连招计数
    if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
    {
        CombatComp->AdvanceComboCount(MaxComboCount);
        CurrentLightAttackComboCount = CombatComp->GetCurrentComboCount();
        UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, 
            TEXT("[PlayerAttackCombo] Combo count advanced via CombatComponent: %d"), 
            CurrentLightAttackComboCount);
    }
}
```

**修改 EndAbility**：
```cpp
void URPGPlayerAbility_AttackCombo::EndAbility(...)
{
    // ✅ 同步计数到 CombatComponent
    if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
    {
        CombatComp->SetCurrentComboCount(CurrentLightAttackComboCount);
        UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, 
            TEXT("[PlayerAttackCombo] Synced combo count to CombatComponent: %d"), 
            CurrentLightAttackComboCount);
    }
    
    // 启动连招窗口定时器
    StartComboWindowTimer();
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

**修改 StartComboWindowTimer**：
```cpp
void URPGPlayerAbility_AttackCombo::StartComboWindowTimer()
{
    // ✅ 委托给 CombatComponent 处理定时器
    if (UPlayerCombatComponent* CombatComp = GetCombatComponentFromActorInfo())
    {
        AActor* Owner = CombatComp->GetOwner();
        if (!Owner) return;
        
        UWorld* World = Owner->GetWorld();
        if (!World) return;
        
        // 清除旧定时器
        World->GetTimerManager().ClearTimer(ComboCountResetTimerHandle);
        
        // ✅ 创建委托绑定到 CombatComponent 的定时器回调
        FTimerDynamicDelegate TimerDelegate;
        TimerDelegate.BindUFunction(CombatComp, FName("OnComboWindowTimerExpired"));
        
        World->GetTimerManager().SetTimer(
            ComboCountResetTimerHandle,
            TimerDelegate,
            ComboWindowTime,
            false
        );
        
        UE_LOG(LogRPGPlayerAbility_AttackCombo, Log, 
            TEXT("[PlayerAttackCombo] Combo window timer started on CombatComponent (%.2f seconds)"), 
            ComboWindowTime);
    }
}
```

---

## 📊 验证结果

### 成功日志示例

```
Wed Apr 15 14:11:36 CST 2026  Log  [PlayerAttackCombo] Synced combo count from CombatComponent: 1
Wed Apr 15 14:11:36 CST 2026  Log  [PlayerCombatComponent] Combo count advanced to 2
Wed Apr 15 14:11:38 CST 2026  Log  [PlayerAttackCombo] Synced combo count to CombatComponent: 2
Wed Apr 15 14:11:38 CST 2026  Log  [PlayerAttackCombo] Combo window timer started on CombatComponent (3.00 seconds)

# 等待3秒后...

Wed Apr 15 14:11:41 CST 2026  Warning  [PlayerCombatComponent] >>> Combo window timer expired, resetting combo <<<
Wed Apr 15 14:11:41 CST 2026  Log      [PlayerCombatComponent] Combo count reset to 1 (timer expired)
```

### 关键验证点

✅ 定时器在 CombatComponent 上成功启动  
✅ 3秒后定时器回调正确触发  
✅ 连招计数成功重置为 1  
✅ 下次攻击时从 CombatComponent 同步到正确的计数  

---

## 🎯 关键经验教训

### 1. 避免在 Gameplay Ability 中使用 K2_SetTimerDelegate

**规则**：C++ 中应使用 `GetWorld()->GetTimerManager().SetTimer()`

**原因**：K2_ 系列函数是为蓝图设计的，在 C++ 中可能存在兼容性问题

---

### 2. 定时器应绑定到持久化对象

**规则**：定时器回调应绑定到 Component 或 Actor，而不是 Ability 实例

**原因**：
- Ability 实例有特殊的生命周期管理（InstancedPerActor/InstancedPerExecution）
- Component 和 Actor 是持久化对象，生命周期更稳定
- 避免委托在对象状态变化时失效

---

### 3. 定时器回调函数必须有 UFUNCTION 标记

**规则**：使用 `BindUFunction` 时，目标函数必须标记为 `UFUNCTION()`

**原因**：
- `BindUFunction` 通过 UE 反射系统查找函数
- 没有 `UFUNCTION()` 标记的函数不会被反射系统注册
- 导致委托绑定静默失败（编译通过但运行时无效）

---

### 4. 状态管理应遵循单一职责原则

**规则**：
- **Component** 管理状态（数据、定时器）
- **Ability** 管理逻辑（输入处理、动画播放）

**优势**：
- 状态在多个 Ability 间共享
- 避免生命周期问题
- 更符合架构设计原则

---

## 📝 附加功能：PlayerCharacter 调试定时器

为满足调试需求，在 ARPGPlayerCharacter 中添加了 2 秒循环定时器：

```cpp
// BeginPlay 中启动
GetWorld()->GetTimerManager().SetTimer(
    DebugTimerHandle,
    this,
    &ARPGPlayerCharacter::OnDebugTimerTick,
    2.0f,
    true  // 循环
);

// 定时器回调
void ARPGPlayerCharacter::OnDebugTimerTick()
{
    UE_LOG(LogRPGPlayerCharacter, Warning, 
        TEXT("[ARPGPlayerCharacter] Debug Timer Tick - Actor: %s, Location: %s, ComboCount: %d"),
        *GetName(),
        *GetActorLocation().ToString(),
        PlayerCombatComponent ? PlayerCombatComponent->GetCurrentComboCount() : -1);
}
```

**输出示例**：
```
[ARPGPlayerCharacter] Debug Timer Tick - Actor: BP_RPGPlayerCharacter_C_0, Location: X=100.000 Y=200.000 Z=300.000, ComboCount: 1
```

---

## ✅ 总结

本次问题涉及三个层面的错误：

1. **API 使用错误**：使用蓝图函数而非 C++ 原生 API
2. **生命周期问题**：定时器绑定到非持久化对象
3. **反射系统问题**：缺少 UFUNCTION 标记导致委托绑定失败

通过架构重构，将连招状态迁移到 PlayerCombatComponent，不仅解决了定时器问题，还提升了系统的可维护性和扩展性。

**修复完成时间**：2026-04-15  
**测试状态**：✅ 通过
