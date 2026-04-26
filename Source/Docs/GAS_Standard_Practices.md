# UE5 GAS 标准用法与最佳实践

**创建日期**: 2026-04-26  
**参考项目**: RPG(本项目)、Warrior、Aura、Lyra  
**GAS版本**: UE 5.6  
**文档状态**: v1.0 初版

---

## 目录

1. [GAS架构设计哲学](#一gas架构设计哲学)
2. [GameplayAbility标准用法](#二gameplayability标准用法)
3. [GameplayEffect标准用法](#三gameplayeffect标准用法)
4. [AttributeSet标准用法](#四attributeset标准用法)
5. [GameplayTag标准用法](#五gameplaytag标准用法)
6. [Event通信模式](#六event通信模式)
7. [AbilitySystemComponent配置](#七abilitysystemcomponent配置)
8. [ExecutionCalculation标准用法](#八executioncalculation标准用法)
9. [AbilityTask标准用法](#九abilitytask标准用法)
10. [数据驱动设计](#十数据驱动设计)
11. [网络同步规范](#十一网络同步规范)
12. [组件化架构](#十二组件化架构)
13. [动画系统集成](#十三动画系统集成)
14. [项目对比分析](#十四项目对比分析)
15. [常见陷阱与解决方案](#十五常见陷阱与解决方案)

---

## 一、GAS架构设计哲学

### 1.1 核心设计原则

#### 原则1: 数据与逻辑分离

```
❌ 错误做法:
Ability硬编码伤害数值
GE写死属性修改值

✅ 正确做法:
Ability只负责流程控制
GE配置数据结构,运行时传入实际值
DataAsset/DataTable管理静态数据
```

**项目对比**:

| 项目 | 实现方式 | 优点 |
|------|---------|------|
| **RPG** | FScalableFloat + DataAsset | 等级缩放,配置灵活 |
| **Warrior** | DataAsset + Blueprint GE | 设计师友好 |
| **Aura** | DataTable + AttributeSet | 多角色管理方便 |
| **Lyra** | PrimaryDataAsset + SoftObjectPtr | 异步加载,内存优化 |

---

#### 原则2: 事件驱动解耦

```
❌ 错误做法:
CombatComponent直接调用Enemy.Die()
AttributeSet直接引用GameMode

✅ 正确做法:
CombatComponent发送Event
被动GA监听Event并处理
AttributeSet设置Tag,其他系统监听
```

**Warrior实现**:
```
碰撞检测 → SendGameplayEventToActor → WaitGameplayEvent Task → HandleApplyDamage → ApplyGE
```

---

#### 原则3: 服务器权威

```
❌ 错误做法:
客户端计算伤害并应用到服务器
客户端直接修改Health属性

✅ 正确做法:
客户端发送Input事件
Server验证并计算伤害
Server应用GE,复制到客户端
```

**Lyra实现**:
- `NetExecutionPolicy = ServerOnly` (关键逻辑)
- `NetSecurityPolicy = ServerOnlyTermination` (防作弊)

---

### 1.2 GAS四大核心组件职责

| 组件 | 职责 | 不应做的事情 |
|------|------|-------------|
| **GameplayAbility** | 流程控制、输入处理、动画播放 | 直接修改属性、硬编码数值 |
| **GameplayEffect** | 属性修改容器、配置数据结构 | 执行复杂逻辑、引用外部系统 |
| **AttributeSet** | 属性存储、验证、网络同步 | 调用游戏逻辑、引用GameMode |
| **AbilitySystemComponent** | 能力管理、效果应用、事件分发 | 存储游戏状态、实现业务逻辑 |

---

## 二、GameplayAbility标准用法

### 2.1 InstancingPolicy选择

#### InstancedPerActor (推荐)

**适用场景**: 
- 需要保存状态(连招计数、冷却时间)
- 需要网络同步
- 多个能力实例可能同时存在

**项目实现**:

```cpp
// RPG项目
URPGPlayerAbility_AttackCombo::URPGPlayerAbility_AttackCombo()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

// Warrior项目 - 相同
// Aura项目 - 相同
// Lyra项目 - 相同
```

**内存占用**: 每个Actor 1个实例 × Ability数量

---

#### InstancedPerExecution

**适用场景**:
- 无状态能力
- 瞬时效果(如一次性治疗)
- 不需要网络同步

**注意**: 预测机制可能失效

---

#### NonInstanced (不推荐)

**适用场景**: 
- 几乎不用
- 仅用于纯工具函数

**UE5.5警告**: `UE_DEPRECATED_FORGAME(5.5, "Use InstancedPerActor as the default")`

---

### 2.2 NetExecutionPolicy选择

| Policy | 执行位置 | 适用场景 | 项目使用 |
|--------|---------|---------|---------|
| **LocalPredicted** | 客户端预测 → Server确认 | 玩家移动、跳跃 | RPG JumpAbility |
| **LocalOnly** | 仅本地执行 | 纯视觉效果 | 无 |
| **ServerInitiated** | Server发起 → 客户端同步 | 敌人AI攻击 | Warrior Enemy |
| **ServerOnly** | 仅Server执行 | 伤害计算、物品拾取 | Lyra核心逻辑 |

**RPG项目配置**:

```cpp
// 跳跃能力 - 需要预测
URPGPlayerAbility_Jump::URPGPlayerAbility_Jump()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

// 攻击能力 - 待确认
URPGPlayerAbility_AttackCombo::URPGPlayerAbility_AttackCombo()
{
    // 当前: ClientOrServer (不安全)
    // 建议: ServerOnly (伤害计算在Server)
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}
```

---

### 2.3 NetSecurityPolicy选择

| Policy | 客户端权限 | 适用场景 |
|--------|-----------|---------|
| **ClientOrServer** | 可执行、可终止 | 非关键逻辑 |
| **ServerOnlyExecution** | 只能请求终止 | 防刷技能 |
| **ServerOnlyTermination** | 可执行、不能终止 | 动画播放中防打断 |
| **ServerOnly** | 完全不能控制 | 核心战斗逻辑 |

**Lyra安全实践**:
```cpp
// 武器射击能力
NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

// 移动能力
NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
```

---

### 2.4 Ability激活方式

#### 方式1: Input激活 (玩家输入)

```cpp
// RPG项目 - InputAction绑定
// RPGPlayerController中:
InputComponent->BindAction(InputAction_Attack, ETriggerEvent::Started, 
    this, &ARPGPlayerController::Input_Attack);

// 调用:
ASC->TryActivateAbility(InputID);
```

**Warrior相同实现**

---

#### 方式2: Event激活 (被动监听)

```cpp
// RPG项目 - 攻击GA监听MeleeHit事件
WaitMeleeHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this,
    RPGGameplayTags::Shared_Event_MeleeHit,
    nullptr, false, true
);
WaitMeleeHitEventTask->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::HandleApplyDamage);
WaitMeleeHitEventTask->ReadyForActivation();
```

**Warrior相同实现 (蓝图)**

---

#### 方式3: Tag激活 (状态变化)

```cpp
// Aura项目 - Tag存在时激活
ActivationOwnedTags.AddTag(Aura.Player.Status.Idle);

// 或: 监听Tag变化
ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
    .AddUObject(this, &UClass::OnTagChanged);
```

---

### 2.5 Ability生命周期管理

```
ActivateAbility
  ↓
执行逻辑 (播放动画、发送事件、应用GE)
  ↓
EndAbility (必须调用!)
  ↓
清理资源 (Task自动清理)
```

**关键规则**:

1. ✅ 必须调用`EndAbility`,否则内存泄漏
2. ✅ Montage回调中调用`EndAbility`
3. ✅ 异常情况下也要调用`EndAbility`
4. ❌ 不要重复调用`EndAbility`

**RPG项目实现**:

```cpp
void URPGPlayerAbility_AttackCombo::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGPlayerAbility_AttackCombo::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
```

---

## 三、GameplayEffect标准用法

### 3.1 GE类型选择

| Duration Policy | 用途 | 示例 | 项目使用 |
|----------------|------|------|---------|
| **Instant** | 瞬时效果 | 伤害、治疗 | ✅ 伤害GE |
| **Infinite** | 持续存在 | Buff、Debuff | ❌ 未实现 |
| **HasDuration** | 限时效果 | 加速、护盾 | ❌ 未实现 |

**RPG伤害GE配置** (待创建蓝图):

```
Duration Policy: Instant
Modifiers:
  - Attribute: DamageTaken
  - Modifier Op: AddBase
  - Magnitude Type: SetByCaller (Shared.SetByCaller.BaseDamage)

Execution:
  - Execution Class: GEExecCale_DamageTaken
  - Source: Player
  - Target: Enemy
```

---

### 3.2 SetByCaller标准用法

#### 为什么用SetByCaller?

```
❌ 错误做法:
GE写死伤害值 = 50
问题: 无法动态计算

✅ 正确做法:
GE配置SetByCaller Tag
运行时传入实际值
```

**RPG项目实现**:

```cpp
// 1. 定义Tag (RPGGameplayTags.h)
UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_BaseDamage, "Shared.SetByCaller.BaseDamage")

// 2. 设置值 (RPGPlayerAbility_AttackCombo.cpp)
SpecHandle.Data->SetSetByCallerMagnitude(
    RPGGameplayTags::Shared_SetByCaller_BaseDamage,
    BaseDamage  // 动态计算的值
);

// 3. ExecCalc读取值 (GEExecCale_DamageTaken.cpp)
float BaseDamage = Spec.GetSetByCallerMagnitude(
    RPGGameplayTags::Shared_SetByCaller_BaseDamage
);
```

---

#### SetByCaller vs Attribute

| 方案 | 适用场景 | 优点 | 缺点 |
|------|---------|------|------|
| **SetByCaller** | 临时传递数据 | 灵活,不污染AS | 需手动传递 |
| **Attribute** | 持久化数据 | 自动同步,可监听 | 增加AS复杂度 |

**Aura项目实践**:
- 伤害值 → SetByCaller
- 攻击力、防御力 → Attribute
- 连招数 → SetByCaller

---

### 3.3 Modifier Op选择

| Modifier Op | 计算公式 | 使用场景 |
|------------|---------|---------|
| **AddBase** | Base + Value | 固定值增加 |
| **Override** | Value | 覆盖原值 |
| **Multiply** | Base × (1 + Value) | 百分比增加 |

**示例**:

```
场景: 装备增加10攻击力
Modifier Op: AddBase
Magnitude: 10

场景: Buff增加20%攻击速度
Modifier Op: Multiply
Magnitude: 0.20

场景: 设置最大生命值为100
Modifier Op: Override
Magnitude: 100
```

---

### 3.4 GE配置检查清单

- [ ] Duration Policy设置正确
- [ ] SetByCaller Magnitudes已添加
- [ ] Execution Calculation已绑定
- [ ] Source和Target已配置
- [ ] Modifiers的Attribute路径正确
- [ ] Modifier Op选择合理
- [ ] 网络复制模式正确 (Server/Client)

---

## 四、AttributeSet标准用法

### 4.1 属性分类标准

| 分类 | 用途 | 示例 | 网络同步 |
|------|------|------|---------|
| **Primary** | 基础属性 | 攻击力、防御力 | 不频繁同步 |
| **Secondary** | 派生属性 | 暴击率、闪避率 | 不频繁同步 |
| **Vital** | 关键属性 | 生命值、法力值 | 频繁同步 |
| **Meta** | 临时属性 | DamageTaken、HealingReceived | 不同步 |

**RPG项目分类**:

```cpp
// Primary
ATTRIBUTE_ACCESSORS(URPGAttributeSet, AttackPower)
ATTRIBUTE_ACCESSORS(URPGAttributeSet, DefensePower)

// Vital
ATTRIBUTE_ACCESSORS(URPGAttributeSet, CurrentHealth)
ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxHealth)

// Meta
ATTRIBUTE_ACCESSORS(URPGAttributeSet, DamageTaken)
```

---

### 4.2 属性验证三阶段

#### 阶段1: PreAttributeChange (修改前)

```cpp
void URPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetCurrentHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
}
```

**用途**: 范围验证、类型转换

---

#### 阶段2: PostAttributeChange (修改后)

```cpp
void URPGAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        // 最大值增加时,当前值也增加
        if (NewValue > OldValue)
        {
            SetCurrentHealth(GetCurrentHealth() + (NewValue - OldValue));
        }
    }
}
```

**用途**: 级联修改、UI更新

---

#### 阶段3: PostGameplayEffectExecute (GE执行后)

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
            // 通过Event或Tag通知,不要直接调用游戏逻辑
        }
    }
}
```

**用途**: 元属性处理、复杂逻辑

---

### 4.3 网络同步规范

#### DOREPLIFETIME配置

```cpp
void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 频繁同步的属性 (Health)
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

    // 不频繁同步的属性 (Attack/Defense)
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, AttackPower, COND_OwnerOnly, REPNOTIFY_Always);
}
```

**同步条件选择**:

| 条件 | 同步对象 | 适用场景 |
|------|---------|---------|
| **COND_None** | 所有人 | 玩家Health (需要显示血条) |
| **COND_OwnerOnly** | 仅Owner | 攻击力 (其他玩家不需要) |
| **COND_SkipOwner** | 除Owner外所有人 | 敌人Health (Owner不需要看到自己的血条) |

---

#### OnRep函数

```cpp
UFUNCTION()
void OnRep_CurrentHealth(float OldValue);

// .cpp
void URPGAttributeSet::OnRep_CurrentHealth(float OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CurrentHealth, OldValue);

    // 触发UI更新
    // 通过Delegate或Event,不要直接引用UI
}
```

**REPNOTIFY_Always的作用**:
- 确保即使值相同也触发OnRep
- 对Health很重要 (可能从100→100,但需要刷新UI)

---

### 4.4 AttributeSet设计禁忌

| ❌ 禁止 | ✅ 正确做法 |
|--------|-----------|
| 引用GameMode/Controller | 发送Event或设置Tag |
| 直接调用Die() | 设置Dead Tag,由其他系统监听 |
| 播放动画/音效 | 通过Event通知AnimationComponent |
| 修改其他AttributeSet | 通过GE间接修改 |

**Warrior项目实践**:
```cpp
// ❌ 错误
void UWarriorAttributeSet::PostGameplayEffectExecute(...)
{
    if (Health <= 0)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Data.Target.GetAvatarActor());
        Enemy->Die();  // 直接引用!
    }
}

// ✅ 正确
void UWarriorAttributeSet::PostGameplayEffectExecute(...)
{
    if (Health <= 0)
    {
        ASC->AddLooseGameplayTag(Shared_Status_Dead);
        // 由EnemyCharacter监听Tag变化并调用Die()
    }
}
```

---

## 五、GameplayTag标准用法

### 5.1 NativeGameplayTags定义

#### C++定义 (推荐)

```cpp
// RPGGameplayTags.h
namespace RPGGameplayTags
{
    // Input Tags
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Light);
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Heavy);

    // Player Tags
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Status_Jumping);
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Weapon_Sword);

    // Shared Tags
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_MeleeHit);
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_Dead);

    // SetByCaller Tags
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_SetByCaller_BaseDamage);
}

// RPGGameplayTags.cpp
UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Light, "InputTag.Attack.Light")
UE_DEFINE_GAMEPLAY_TAG(Shared_Event_MeleeHit, "Shared.Event.MeleeHit")
```

**优点**:
- ✅ 编译期检查 (拼写错误编译失败)
- ✅ 自动补全 (IDE支持)
- ✅ 性能更好 (不需要字符串查找)

---

#### 蓝图定义 (不推荐)

```
 GameplayTagList
   - Tag: InputTag.Attack.Light
   - Tag: Shared.Event.MeleeHit
```

**缺点**:
- ❌ 运行时检查 (拼写错误运行时才发现)
- ❌ 无法自动补全
- ❌ 容易拼写错误

---

### 5.2 Tag命名空间规范

| 命名空间 | 用途 | 示例 |
|---------|------|------|
| **InputTag** | 输入动作 | InputTag.Attack.Light |
| **Player** | 玩家专属 | Player.Status.Jumping |
| **Enemy** | 敌人专属 | Enemy.Status.Stunned |
| **Shared** | 共享标签 | Shared.Event.MeleeHit |
| **Ability** | 能力类型 | Ability.Attack.Combo |
| **SetByCaller** | 动态传参 | Shared.SetByCaller.BaseDamage |
| **Data** | 数据标签 | Data.Weapon.Sword |

**Lyra项目命名**:
```
Lyra.Activity.Firing
Lyra.Weapon.Shotgun
Lyra.Gameplay.Damage
```

---

### 5.3 Tag使用场景

#### 场景1: 能力激活条件

```cpp
// 激活时需要这些Tag不存在
ActivationBlockedTags.AddTag(RPGGameplayTags::Player_Status_Jumping);

// 激活时需要这些Tag存在
ActivationRequiredTags.AddTag(RPGGameplayTags::Player_Status_Grounded);
```

---

#### 场景2: 能力互斥

```cpp
// 给能力添加Tag
AbilityTags = FGameplayTagContainer(RPGGameplayTags::Ability_Attack);

// 阻塞相同Tag的能力
ActivationBlockedTags.AddTag(RPGGameplayTags::Ability_Attack);
```

**效果**: 攻击中不能再次攻击

---

#### 场景3: 状态监听

```cpp
// 注册Tag变化事件
ASC->RegisterGameplayTagEvent(
    RPGGameplayTags::Shared_Status_Dead,
    EGameplayTagEventType::NewOrRemoved
).AddUObject(this, &ARPGEnemyCharacter::OnDeadTagChanged);
```

---

## 六、Event通信模式

### 6.1 Event数据传递标准

#### FGameplayEventData结构

```cpp
struct FGameplayEventData
{
    FGameplayTag EventTag;              // 事件标签
    const AActor* Instigator;          // 发起者
    const AActor* Target;              // 目标
    const UObject* OptionalObject;     // 自定义对象
    const UObject* OptionalObject2;    // 第二个自定义对象
    FGameplayEffectContextHandle ContextHandle;  // 上下文
    FGameplayTagContainer InstigatorTags;        // 发起者Tag
    FGameplayTagContainer TargetTags;            // 目标Tag
    float EventMagnitude;                        // 数值
    FGameplayAbilityTargetDataHandle TargetData; // 目标数据
};
```

**最小传递原则** (Warrior标准):
```cpp
// ✅ 正确: 只传必要信息
Data.Instigator = GetOwningPawn();
Data.Target = HitActor;

// ❌ 错误: 传递过多数据
Data.EventMagnitude = Damage;  // 应由GA自行查询
Data.OptionalObject = CustomData;  // 避免
```

---

### 6.2 Event发送与接收

#### 发送Event

```cpp
// RPG项目 - 碰撞检测后发送
UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
    GetOwningPawn(),                    // 发送给攻击者自己
    RPGGameplayTags::Shared_Event_MeleeHit,
    Data
);
```

**关键点**:
- ✅ 发送给攻击者自己的ASC (不是目标)
- ✅ 被动GA在攻击者身上监听
- ✅ GA应用GE到目标

---

#### 接收Event

```cpp
// RPG项目 - 攻击GA中监听
UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this,
    RPGGameplayTags::Shared_Event_MeleeHit,
    nullptr, false, true
);
Task->EventReceived.AddDynamic(this, &URPGPlayerAbility_AttackCombo::HandleApplyDamage);
Task->ReadyForActivation();
```

**委托签名**:
```cpp
// ✅ 正确: 按值传递
UFUNCTION()
void HandleApplyDamage(FGameplayEventData EventData);

// ❌ 错误: const引用会导致委托绑定失败
void HandleApplyDamage(const FGameplayEventData& EventData);
```

---

### 6.3 Event通信架构对比

#### 架构A: 直接调用 (❌ 不推荐)

```
CombatComponent → Target.TakeDamage() → Target.Die()

问题: 强耦合,无法扩展
```

#### 架构B: Event驱动 (✅ Warrior标准)

```
CombatComponent → SendEvent → 被动GA监听 → ApplyGE → PostExecute → SetTag → 监听Tag → Die()

优点: 完全解耦,易于扩展
```

#### 架构C: Delegate回调 (部分场景)

```
Weapon.OnHit.AddDynamic(this, &CombatComponent::OnHitTarget)

适用: 组件内部通信,不涉及GAS
```

---

## 七、AbilitySystemComponent配置

### 7.1 复制模式选择

| 模式 | 适用对象 | 同步内容 | 带宽占用 |
|------|---------|---------|---------|
| **Full** | 玩家角色 | 所有属性、能力、效果 | 高 |
| **Mixed** | NPC | 部分属性 | 中 |
| **Minimal** | 小怪、道具 | 最小化 | 低 |

**RPG项目配置** (待确认):

```cpp
// 玩家 - Full复制
URPGAbilitySystemComponent* RPGASC = NewObject<URPGAbilitySystemComponent>(
    PlayerState,
    URPGAbilitySystemComponent::StaticClass()
);
RPGASC->SetIsReplicated(true);

// 敌人 - Minimal复制 (建议)
EnemyASC->SetIsReplicated(true);
// 默认Minimal,无需额外配置
```

---

### 7.2 InitAbilityActorInfo时机

| 角色类型 | 初始化时机 | 函数 |
|---------|-----------|------|
| **Player** | PossessedBy | `PlayerState->InitAbilityActorInfo()` |
| **Enemy** | BeginPlay | `EnemyCharacter->InitAbilityActorInfo()` |
| **AI** | Spawn时 | `EnemyController->InitAbilityActorInfo()` |

**RPG项目实现**:

```cpp
// 玩家 - RPGPlayerState.cpp
void ARPGPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
    }
}

// 敌人 - RPGEnemyCharacter.cpp
void ARPGEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    if (RPGAbilitySystemComponent)
    {
        FGameplayAbilityActorInfo* ActorInfo = new FGameplayAbilityActorInfo();
        ActorInfo->InitFromActor(this, this, RPGAbilitySystemComponent);
        RPGAbilitySystemComponent->InitAbilityActorInfo(ActorInfo);
    }
}
```

---

### 7.3 能力授予方式

#### 方式1: StartupData (推荐)

```cpp
// RPGPlayerDataAsset.h
UPROPERTY(EditDefaultsOnly, Category = "Abilities")
TArray<FGameplayAbilitySpecDef> StartupAbilities;

// 授予时
for (const FGameplayAbilitySpecDef& AbilityDef : StartupAbilities)
{
    FGameplayAbilitySpec Spec(AbilityDef.Ability, AbilityDef.Level);
    AbilitySystemComponent->GiveAbility(Spec);
}
```

**优点**:
- ✅ 配置化,无需硬编码
- ✅ 支持等级缩放
- ✅ Designer友好

---

#### 方式2: 动态授予

```cpp
// 武器能力 - 装备时授予
void URPGPlayerAbility_EquipSword::GrantWeaponAbilities()
{
    for (TSubclassOf<UGameplayAbility> AbilityClass : WeaponAbilities)
    {
        FGameplayAbilitySpec Spec(AbilityClass);
        ASC->GiveAbility(Spec);
    }
}
```

---

## 八、ExecutionCalculation标准用法

### 8.1 ExecCalc职责

```
✅ 应该做的:
- 捕获源和目标属性
- 执行复杂计算
- 输出到Meta属性

❌ 不应该做的:
- 修改非Meta属性
- 调用游戏逻辑
- 引用外部系统
```

---

### 8.2 ExecCalc实现标准

#### 步骤1: 定义捕获

```cpp
// GEExecCale_DamageTaken.h
struct FDamageTakenStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);    // 源属性
    DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);   // 目标属性
    DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken);    // 目标Meta属性
};
```

---

#### 步骤2: 注册捕获

```cpp
// 构造函数中
FDamageTakenStatics& DamageStatics()
{
    static FDamageTakenStatics* Statics = new FDamageTakenStatics();
    return *Statics;
}

UGEExecCale_DamageTaken::UGEExecCale_DamageTaken()
{
    // 源属性 (攻击者)
    DefineAttributeCaptureDef(DamageStatics().AttackPower, EGameplayEffectAttributeCaptureSource::Source, false);
    
    // 目标属性 (受击者)
    DefineAttributeCaptureDef(DamageStatics().DefensePower, EGameplayEffectAttributeCaptureSource::Target, false);
    DefineAttributeCaptureDef(DamageStatics().DamageTaken, EGameplayEffectAttributeCaptureSource::Target, false);
}
```

---

#### 步骤3: 执行计算

```cpp
void UGEExecCale_DamageTaken::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // 1. 捕获属性
    FAggregatorEvaluateParameters Params;
    Params.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    Params.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float SourceAttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().AttackPowerDef, Params, SourceAttackPower
    );

    float TargetDefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().DefensePowerDef, Params, TargetDefensePower
    );

    // 2. 读取SetByCaller
    float BaseDamage = Spec.GetSetByCallerMagnitude(RPGGameplayTags::Shared_SetByCaller_BaseDamage);

    // 3. 计算
    float FinalDamage = BaseDamage * SourceAttackPower / TargetDefensePower;

    // 4. 输出
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            DamageStatics().DamageTaken,
            EGameplayModOp::Override,
            FinalDamage
        )
    );
}
```

---

### 8.3 ExecCalc vs PostExecute分工

| 阶段 | 职责 | 示例 |
|------|------|------|
| **ExecCalc** | 复杂计算 | 伤害公式、属性比例 |
| **PostExecute** | 简单处理 | 扣血、死亡检测、触发事件 |

**RPG项目实现**:

```cpp
// ExecCalc: 计算最终伤害
FinalDamage = BaseDamage × ComboMultiplier × (AttackPower / DefensePower)

// PostExecute: 应用伤害
CurrentHealth = Clamp(OldHealth - FinalDamage, 0, MaxHealth)
if (CurrentHealth <= 0) → 触发死亡
```

---

## 九、AbilityTask标准用法

### 9.1 常用AbilityTask

| Task | 用途 | 项目使用 |
|------|------|---------|
| **PlayMontageAndWait** | 播放蒙太奇并等待回调 | ✅ 攻击GA |
| **WaitGameplayEvent** | 监听GAS事件 | ✅ 攻击GA |
| **WaitDelay** | 延迟执行 | ❌ 未使用 |
| **WaitAttributeChange** | 监听属性变化 | ❌ 未使用 |
| **WaitTargetData** | 等待目标选择 | ❌ 未使用 |

---

### 9.2 PlayMontageAndWait标准用法

```cpp
UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    this,
    NAME_None,              // Task名称
    MontageToPlay,          // 蒙太奇资产
    1.0f,                   // 播放速率
    FName(),                // 起始Section
    0.0f,                   // 起始时间
    true                    // 停止时播放结束动画
);

if (MontageTask)
{
    MontageTask->OnCompleted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCompleted);
    MontageTask->OnBlendOut.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageBlendOut);
    MontageTask->OnInterrupted.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &URPGPlayerAbility_AttackCombo::OnMontageCancelled);
    MontageTask->ReadyForActivation();  // 必须调用!
}
```

**回调区别**:

| 回调 | 触发时机 | 应调用EndAbility |
|------|---------|-----------------|
| **OnCompleted** | 动画自然播放完毕 | ✅ bReplicateEndAbility = true |
| **OnBlendOut** | 动画淡出 | ✅ (通常与Completed同时) |
| **OnInterrupted** | 被其他动画打断 | ✅ bWasCancelled = true |
| **OnCancelled** | Ability被取消 | ✅ bWasCancelled = true |

---

### 9.3 WaitGameplayEvent标准用法

```cpp
UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this,                       // Owning Ability
    EventTag,                   // 事件Tag
    nullptr,                    // Optional Actor Filter
    false,                      // bOnlyTriggerOnce
    true                        // bMatchAny (Tag匹配模式)
);

if (Task)
{
    // ✅ 正确: 按值传递
    Task->EventReceived.AddDynamic(this, &UClass::OnEventReceived);
    Task->ReadyForActivation();
}
```

**常见错误**:

```cpp
// ❌ 错误1: 委托签名不匹配
Task->EventReceived.AddDynamic(this, &UClass::OnEventReceived(const FGameplayEventData&));

// ✅ 正确
Task->EventReceived.AddDynamic(this, &UClass::OnEventReceived(FGameplayEventData));

// ❌ 错误2: 忘记ReadyForActivation
Task->EventReceived.AddDynamic(this, &UClass::OnEventReceived);
// 缺少: Task->ReadyForActivation();

// ❌ 错误3: 使用ValidData而不是EventReceived
Task->ValidData.AddDynamic(...);  // 不存在的委托!
```

---

## 十、数据驱动设计

### 10.1 DataAsset标准用法

#### RPG项目实现

```cpp
// RPGPlayerDataAsset.h
UCLASS()
class RPG_API URPGPlayerDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Attributes")
    FScalableFloat PlayerLevel = FScalableFloat(1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TArray<FGameplayAbilitySpecDef> StartupAbilities;

    UPROPERTY(EditDefaultsOnly, Category = "Attributes")
    TSubclassOf<UAttributeSet> AttributeSetClass;
};
```

**FScalableFloat结构**:
```cpp
struct FScalableFloat
{
    UCurveFloat* Curve;     // 等级曲线
    float Scale;            // 缩放系数
    
    float GetValueAtLevel(float Level) const
    {
        return Curve ? Curve->GetFloatValue(Level) * Scale : Scale;
    }
};
```

---

### 10.2 DataTable vs DataAsset

| 方案 | 适用场景 | 优点 | 缺点 | 项目使用 |
|------|---------|------|------|---------|
| **DataTable** | 多角色属性表 | Excel导入,批量管理 | 类型固定 | Aura |
| **DataAsset** | 单个角色配置 | 灵活,支持复杂结构 | 需手动创建 | Warrior, RPG |
| **PrimaryDataAsset** | 需要异步加载 | 支持软引用 | 复杂度高 | Lyra |

**Aura项目DataTable**:
```
Row: Hero_Warrior
  - MaxHealth: 500
  - AttackPower: 50
  - DefensePower: 30

Row: Hero_Mage
  - MaxHealth: 300
  - AttackPower: 80
  - DefensePower: 20
```

**RPG项目DataAsset**:
```cpp
BP_DataAsset_Player
  ├─ StartupAbilities: [GA_Attack, GA_Jump, GA_EquipSword]
  ├─ PlayerLevel: FScalableFloat (Curve_Level, Scale=1.0)
  └─ AttributeSetClass: URPGAttributeSet
```

---

### 10.3 异步加载标准

```cpp
// Lyra项目 - 异步加载武器资产
TSoftObjectPtr<UAnimMontage> SoftMontage;

void LoadMontage()
{
    if (!SoftMontage.IsValid())
    {
        SoftMontage.LoadSynchronous();  // 或异步
    }
    UAnimMontage* Montage = SoftMontage.Get();
}
```

**异步vs同步**:

| 方式 | 适用场景 | 优点 | 缺点 |
|------|---------|------|------|
| **LoadSynchronous** | 初始化时 | 简单 | 卡顿 |
| **RequestAsyncLoad** | 运行时 | 不卡顿 | 复杂 |
| **StreamableManager** | 批量加载 | 统一管理 | 需要额外代码 |

---

## 十一、网络同步规范

### 11.1 服务器权威架构

```
客户端:
  1. 玩家输入
  2. 预测本地效果
  3. 发送RPC到Server

服务器:
  1. 验证输入
  2. 执行逻辑
  3. 应用GE
  4. 复制到所有客户端

客户端:
  1. 收到Server状态
  2. 纠正预测误差
```

---

### 11.2 预测机制

#### LocalPredicted执行流程

```cpp
// 客户端预测
Client: TryActivateAbility(Jump)
Client: 立即执行跳跃 (预测)
Client: 发送ServerTryActivateAbility RPC

// Server确认
Server: 收到RPC
Server: 验证CanActivateAbility
Server: 执行跳跃
Server: 复制状态到Client

// 客户端纠正
Client: 收到Server状态
Client: 如果预测错误,纠正位置
```

**RPG JumpAbility配置**:
```cpp
URPGPlayerAbility_Jump::URPGPlayerAbility_Jump()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
```

---

### 11.3 网络优化技巧

#### 技巧1: 属性复制条件

```cpp
// 玩家Health - 所有人同步 (血条显示)
DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);

// 敌人Health - 仅同步给非Owner
DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CurrentHealth, COND_SkipOwner, REPNOTIFY_Always);

// 攻击力 - 仅Owner需要同步
DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, AttackPower, COND_OwnerOnly, REPNOTIFY_Always);
```

---

#### 技巧2: Minimal复制模式

```cpp
// 敌人使用Minimal复制
EnemyASC->SetIsReplicated(true);
// 默认复制模式: Minimal

// 玩家使用Full复制
PlayerASC->SetReplicationMode(EGameplayEffectReplicationMode::Full);
```

---

#### 技巧3: 批量RPC

```cpp
// Warrior项目 - 批量发送伤害事件
FScopedServerAbilityRPCBatcher Batcher(ASC, AbilityHandle);

ApplyGE1();
ApplyGE2();
SendEvent1();
// 离开作用域时自动批量发送
```

---

## 十二、组件化架构

### 12.1 组件职责划分

| 组件 | 职责 | 不应做的事情 |
|------|------|-------------|
| **CombatComponent** | 碰撞检测、连招管理 | 计算伤害、应用GE |
| **UIComponent** | 血条更新、伤害数字 | 修改属性 |
| **MovementComponent** | 移动逻辑、跳跃 | 播放动画 |
| **AnimationComponent** | 动画播放、蒙太奇 | 处理输入 |

---

### 12.2 组件通信模式

#### 模式1: 通过Owner Pawn获取

```cpp
// RPG项目
UPlayerCombatComponent* URPGPlayerAbility_AttackCombo::GetCombatComponentFromActorInfo() const
{
    if (CachedCombatComponent.IsValid())
        return CachedCombatComponent.Get();

    if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
    {
        if (ARPGPlayerCharacter* Player = Cast<ARPGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
        {
            return Player->GetPlayerCombatComponent();
        }
    }
    return nullptr;
}
```

---

#### 模式2: Interface解耦

```cpp
// Warrior项目
UINTERFACE()
class UCombatInterface : public UInterface
{
    GENERATED_BODY()
};

class ICombatInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void TakeDamage(float Damage) = 0;
};

// 调用
if (ICombatInterface* CombatActor = Cast<ICombatInterface>(Target))
{
    CombatActor->TakeDamage(Damage);
}
```

---

## 十三、动画系统集成

### 13.1 AnimNotify触发Event

```cpp
// 动画通知蓝图中
void UAN_SendGameplayEventToOwner::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (!ASC) return;

    FGameplayEventData Data;
    Data.Instigator = Owner;
    Data.Target = Owner;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        Owner,
        EventTag,
        Data
    );
}
```

**RPG项目使用场景**:
- `AN_AttackHit` → 触发武器碰撞检测
- `AN_JumpFinish` → 发送跳跃完成事件

---

### 13.2 AnimInstance访问GAS

```cpp
// RPGBaseAnimInstance.h
UCLASS()
class URPGBaseAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "GAS")
    bool bHasJumpingTag = false;

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};

// .cpp
void URPGBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (AActor* Owner = GetOwningActor())
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
        {
            bHasJumpingTag = ASC->HasMatchingGameplayTag(RPGGameplayTags::Player_Status_Jumping);
        }
    }
}
```

**用途**: 动画混合、状态机切换

---

## 十四、项目对比分析

### 14.1 架构对比

| 特性 | RPG | Warrior | Aura | Lyra |
|------|-----|---------|------|------|
| **GA实现** | C++ | 蓝图 | 蓝图+C++ | C++ |
| **GE配置** | 待创建蓝图 | 蓝图 | 蓝图 | C++ |
| **ExecCalc** | C++ | C++ | C++ | C++ |
| **数据驱动** | DataAsset | DataAsset | DataTable | PrimaryDataAsset |
| **网络同步** | 部分实现 | 完整 | 完整 | 完整 |
| **组件化** | 中等 | 高 | 中等 | 极高 |
| **动画集成** | 部分 | 完整 | 部分 | 完整 |

---

### 14.2 优势对比

#### RPG项目优势
- ✅ C++实现,性能更好
- ✅ ExecCalc完整
- ✅ 连招系统完善
- ✅ DataAsset配置灵活

#### Warrior项目优势
- ✅ 蓝图GA,Designer友好
- ✅ Event通信完整
- ✅ 组件化程度高
- ✅ Interface解耦

#### Aura项目优势
- ✅ DataTable管理多角色
- ✅ 属性系统完整
- ✅ UI集成完善
- ✅ 教程详细

#### Lyra项目优势
- ✅ 网络同步完整
- ✅ 异步加载优化
- ✅ 模块化架构
- ✅ 生产级代码

---

### 14.3 学习建议

| 学习主题 | 推荐项目 | 原因 |
|---------|---------|------|
| **GAS基础** | Aura | 教程详细,适合入门 |
| **战斗系统** | Warrior | Event通信标准 |
| **网络同步** | Lyra | 生产级实现 |
| **性能优化** | RPG (本项目) | C++实现,可控性强 |

---

## 十五、常见陷阱与解决方案

### 15.1 编译期陷阱

#### 陷阱1: 委托签名不匹配

```cpp
// ❌ 错误
Task->EventReceived.AddDynamic(this, &UClass::OnEvent(const FGameplayEventData&));

// ✅ 正确
Task->EventReceived.AddDynamic(this, &UClass::OnEvent(FGameplayEventData));
```

**原因**: GAS委托按值传递,不支持引用

---

#### 陷阱2: 头文件前向声明

```cpp
// ❌ 错误 - 编译失败
class UAbilityTask_WaitGameplayEvent;  // 前向声明

UAbilityTask_WaitGameplayEvent* Task;  // 无法访问成员

// ✅ 正确
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
```

**规则**: 
- 声明指针/引用 → 前向声明即可
- 访问成员/调用函数 → 必须Include

---

### 15.2 运行期陷阱

#### 陷阱3: EndAbility未调用

```cpp
// ❌ 错误 - 内存泄漏
void URPGAbility::OnMontageCompleted()
{
    // 忘记调用EndAbility
}

// ✅ 正确
void URPGAbility::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

**症状**: 
- Ability实例不断增加
- 内存持续增长

---

#### 陷阱4: Event发送给错误目标

```cpp
// ❌ 错误 - 发送给目标
UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
    TargetActor,  // 错误!
    EventTag,
    Data
);

// ✅ 正确 - 发送给攻击者自己
UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
    GetOwningPawn(),  // 正确!
    EventTag,
    Data
);
```

**原因**: 被动GA在攻击者ASC上监听

---

### 15.3 网络陷阱

#### 陷阱5: 客户端修改属性

```cpp
// ❌ 错误 - 客户端直接修改
CurrentHealth = 100;  // 不会同步到Server

// ✅ 正确 - 通过GE修改
ApplyGameplayEffect(GE_Heal);  // Server执行,自动同步
```

---

#### 陷阱6: 预测未配置

```cpp
// ❌ 错误 - 跳跃能力使用ServerOnly
NetExecutionPolicy = ServerOnly;
// 结果: 客户端有明显延迟

// ✅ 正确 - 使用LocalPredicted
NetExecutionPolicy = LocalPredicted;
// 结果: 立即响应,Server确认
```

---

## 十六、总结与展望

### 16.1 当前项目完成度

| 模块 | 完成度 | 下一步 |
|------|--------|--------|
| **GameplayAbility** | 85% | 完善网络配置 |
| **GameplayEffect** | 50% | 创建蓝图GE |
| **AttributeSet** | 95% | 添加死亡检测 |
| **ExecCalc** | 100% | ✅ 完成 |
| **Event通信** | 80% | 完善HitReact处理 |
| **网络同步** | 60% | 配置复制模式 |
| **组件化** | 85% | 添加Interface |
| **动画集成** | 70% | 完善AnimNotify |

---

### 16.2 最佳实践总结

1. ✅ **InstancedPerActor** 作为默认InstancingPolicy
2. ✅ **SetByCaller** 传递动态数据
3. ✅ **ExecCalc** 处理复杂计算
4. ✅ **Event驱动** 解耦组件
5. ✅ **DataAsset** 配置化设计
6. ✅ **NativeGameplayTags** 编译期检查
7. ✅ **Server权威** 防止作弊
8. ✅ **EndAbility** 必须调用

---

### 16.3 参考资料

- [Unreal官方GAS文档](https://docs.unrealengine.com/)
- [GAS Community Wiki](https://gasetutorials.com/)

---

**文档版本**: v1.0  
**最后更新**: 2026-04-26  
**维护者**: AI Assistant  
**审核状态**: 初稿,待完善
