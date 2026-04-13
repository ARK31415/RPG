# StartupData 系统使用指南

## 概述

StartupData 系统用于在角色（玩家或敌人）生成时自动授予初始能力（Abilities）和效果（Gameplay Effects）。这是一个数据驱动的系统，通过 DataAsset 配置，无需编写额外代码。

## 架构设计

### 类层次结构

```
UDataAsset_StartUpDataBase (基类)
├── UDataAsset_PlayerStartUpData (玩家专用)
└── UDataAsset_EnemyStartUpData (敌人专用)
```

### 核心功能

**UDataAsset_StartUpDataBase** 提供三个主要配置项：

1. **ActiveOnGivenAbilities**: 立即授予的主动能力
2. **ReactiveAbilities**: 被动/响应式能力
3. **StartUpGameplayEffect**: 启动时应用的效果（如初始属性）

**UDataAsset_PlayerStartUpData** 额外提供：

4. **PlayerStartUpAbilitySet**: 带输入标签的能力集合（用于输入绑定）

## 使用方法

### 1. 创建玩家 StartupData

在内容浏览器中：
1. 右键 → Miscellaneous → Data Asset
2. 选择 `DataAsset_PlayerStartUpData`
3. 命名为 `DA_PlayerStartUp_Default`

配置示例：
```
ActiveOnGivenAbilities:
  - GA_Player_Attack (攻击能力)
  - GA_Player_Dodge (闪避能力)

ReactiveAbilities:
  - GA_Player_HitReact (受击反应)
  - GA_Player_Death (死亡能力)

StartUpGameplayEffect:
  - GE_Player_InitialStats (初始属性效果)

PlayerStartUpAbilitySet:
  - AbilityToGrant: GA_Player_LightAttack
    InputTag: InputTag.LightAttack
  - AbilityToGrant: GA_Player_HeavyAttack
    InputTag: InputTag.HeavyAttack
```

### 2. 应用到玩家

在编辑器中打开你的 PlayerState 蓝图或 C++ 默认值：

**C++ 方式**（在 ARPGPlayerCharacter 或 GameMode 中设置）：
```cpp
// 在 Character 蓝图中设置
ARPGPlayerState* PlayerState = GetPlayerState<ARPGPlayerState>();
// 通过编辑器设置 PlayerStartUpData 属性
```

**蓝图方式**：
1. 打开 PlayerState 蓝图（或继承 BP_RPGPlayerState）
2. 在 Details 面板找到 "RPG|Startup" 分类
3. 设置 `PlayerStartUpData` 为你创建的 DataAsset

### 3. 创建敌人 StartupData

同样步骤，但选择 `DataAsset_EnemyStartUpData`：

```
ActiveOnGivenAbilities:
  - GA_Enemy_MeleeAttack (近战攻击)
  - GA_Enemy_RangedAttack (远程攻击)

ReactiveAbilities:
  - GA_Enemy_HitReact (受击反应)
  - GA_Enemy_Death (死亡能力)

StartUpGameplayEffect:
  - GE_Enemy_InitialStats (敌人初始属性)
  - GE_Enemy_ArmorBuff (护甲增益)
```

### 4. 应用到敌人

在你的敌人蓝图（继承自 AEnemyCharacter）中：

1. 打开敌人蓝图
2. 在 Details 面板找到 "RPG|Startup" 分类
3. 设置 `EnemyStartUpData` 为你创建的 DataAsset

## 工作流程

### 玩家初始化流程

```
1. 玩家生成
   ↓
2. ARPGPlayerState::BeginPlay()
   ↓
3. InitAbilityActorInfo(this, GetPawn())
   ↓
4. InitializeStartupData()
   ↓
5. PlayerStartUpData->GiveToAbilitySystemComponent()
   ↓
6. 授予所有能力和效果
   ↓
7. 玩家可以立即使用技能
```

### 敌人初始化流程

```
1. 敌人生成
   ↓
2. AEnemyCharacter::BeginPlay()
   ↓
3. InitAbilityActorInfo(this, this)
   ↓
4. InitializeStartupData()
   ↓
5. EnemyStartUpData->GiveToAbilitySystemComponent()
   ↓
6. 授予所有能力和效果
   ↓
7. AI 可以立即使用技能
```

## 高级用法

### 动态等级缩放

```cpp
// 在 RPGPlayerState 中可以修改 ApplyLevel
void ARPGPlayerState::InitializeStartupData()
{
    if (!PlayerStartUpData || !RPGAbilitySystemComponent)
        return;

    // 根据玩家等级动态调整能力等级
    int32 PlayerLevel = GetPlayerLevel(); // 假设你有这个方法
    PlayerStartUpData->GiveToAbilitySystemComponent(RPGAbilitySystemComponent, PlayerLevel);
}
```

### 条件性授予能力

在自定义的 StartupData 子类中重写 `GiveToAbilitySystemComponent`：

```cpp
void UDataAsset_CustomStartUpData::GiveToAbilitySystemComponent(
    URPGAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
    Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

    // 添加自定义逻辑
    if (ShouldGrantBonusAbility())
    {
        FGameplayAbilitySpec Spec(BonusAbilityClass);
        Spec.Level = ApplyLevel;
        InASCToGive->GiveAbility(Spec);
    }
}
```

### 运行时重新应用

如果需要在中途重新应用 StartupData（如转职、升级）：

```cpp
// 清除旧能力
ASC->ClearAllAbilities();

// 重新应用新配置
NewStartUpData->GiveToAbilitySystemComponent(ASC, NewLevel);
```

## 注意事项

### ⚠️ 重要提醒

1. **执行顺序**：StartupData 在 `BeginPlay()` 中应用，确保 ASC 已正确初始化
2. **网络复制**：能力的授予会自动通过网络复制到客户端
3. **Avatar 设置**：
   - 玩家：`InitAbilityActorInfo(PlayerState, PlayerCharacter)`
   - 敌人：`InitAbilityActorInfo(EnemyCharacter, EnemyCharacter)`
4. **不要重复授予**：避免在多个地方调用 `GiveToAbilitySystemComponent`

### ✅ 最佳实践

1. **模块化设计**：为不同类型的角色创建不同的 StartupData
   - `DA_PlayerStartUp_Warrior`
   - `DA_PlayerStartUp_Mage`
   - `DA_EnemyStartUp_Goblin`
   - `DA_EnemyStartUp_Dragon`

2. **复用基础效果**：创建通用的 GameplayEffect
   - `GE_Base_Health`
   - `GE_Base_MovementSpeed`

3. **版本控制**：StartupData 是 DataAsset，易于版本管理和策划配置

4. **调试支持**：在 GrantAbilities 中添加日志
   ```cpp
   UE_LOG(LogTemp, Log, TEXT("Granted ability: %s"), *Ability->GetName());
   ```

## 故障排除

### 问题：能力未授予

**检查清单**：
1. ✅ StartupData 是否正确分配到 PlayerState/EnemyCharacter
2. ✅ ASC 是否正确初始化（InitAbilityActorInfo 是否调用）
3. ✅ Ability 类是否有效（不为 None）
4. ✅ 检查 Output Log 是否有错误信息

### 问题：客户端不同步

**解决方案**：
- 确保 ASC 设置了 `SetIsReplicated(true)`
- 检查网络连接是否正常
- 验证 Server 端是否正确授予能力

### 问题：能力无法激活

**可能原因**：
1. InputTag 未正确配置（针对 PlayerStartUpAbilitySet）
2. 能力的 Cost 或 Cooldown 阻止了激活
3. 缺少必要的前置条件（Tags）

## 示例项目结构

```
Content/
└── RPG/
    ├── DataAsset/
    │   └── StartUpDate/
    │       ├── DA_PlayerStartUp_Default
    │       ├── DA_PlayerStartUp_Warrior
    │       ├── DA_PlayerStartUp_Mage
    │       ├── DA_EnemyStartUp_Goblin
    │       └── DA_EnemyStartUp_Boss
    ├── Abilities/
    │   ├── GA_Player_Attack
    │   ├── GA_Player_Dodge
    │   ├── GA_Enemy_MeleeAttack
    │   └── ...
    └── GameplayEffects/
        ├── GE_Player_InitialStats
        ├── GE_Enemy_InitialStats
        └── ...
```

## 扩展建议

1. **添加音效/特效**：在授予能力时播放特殊效果
2. **成就系统**：追踪玩家获得的能力
3. **教程系统**：首次获得能力时显示提示
4. **平衡测试**：快速切换不同的 StartupData 进行数值测试

## 相关文档

- [ASC混合架构设计](./ASC混合架构设计.md)
- [Unreal Engine GAS 官方文档](https://docs.unrealengine.com/en-US/GameplayAbilitySystem/)
