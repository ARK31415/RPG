# RPG项目技能配置指南 - 以 EquipSword 为例

## 概述

本文档详细说明如何在RPG项目中为玩家配置一个完整的技能，以 `RPGPlayerAbility_EquipSword`（装备剑技能）为例，从GameplayTag配置到输入绑定、数据资产设置的完整流程。

---

## 配置流程总览

```
1. 定义 GameplayTag（C++）
   ↓
2. 创建 InputAction 资源（蓝图）
   ↓
3. 配置 InputConfig DataAsset（蓝图）
   ↓
4. 创建/配置技能蓝图（蓝图）
   ↓
5. 配置武器数据（蓝图）
   ↓
6. 配置 StartupData（蓝图）
   ↓
7. 验证与测试
```

---

## 第一步：定义 GameplayTag

### 1.1 在 C++ 中声明 Tag

打开文件：`Source/RPG/Public/RPGGameplayTags.h`

在 `RPGGameplayTags` 命名空间中添加你的 Tag 声明：

```cpp
namespace RPGGameplayTags
{
    // Input Tags - 用于输入绑定
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EquipSword);
    
    // Player Tags - 用于标识技能
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Equip_Sword);
    
    // Event Tags - 用于动画通知事件
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Equip_Sword);
    
    // Weapon Tags - 用于标识武器类型
    RPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Weapon_Sword);
}
```

### 1.2 在 C++ 中定义 Tag

打开文件：`Source/RPG/Private/RPGGameplayTags.cpp`

添加 Tag 的具体定义：

```cpp
namespace RPGGameplayTags
{
    // Input Tags
    UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipSword, "InputTag.EquipSword");
    
    // Player Tags
    UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Sword, "Player.Ability.Equip.Sword");
    
    // Event Tags
    UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Sword, "Player.Event.Equip.Sword");
    
    // Weapon Tags
    UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Sword, "Player.Weapon.Sword");
}
```

**命名规范：**
- `InputTag.*` - 输入标签，用于输入绑定
- `Player.Ability.*` - 玩家技能标签，用于标识技能
- `Player.Event.*` - 事件标签，用于动画通知和 GameplayEvent
- `Player.Weapon.*` - 武器标签，用于标识武器类型

### 1.3 重新编译项目

在 Unreal Editor 中点击 **编译** 按钮，或使用 Visual Studio 编译项目。

---

## 第二步：创建 InputAction 资源

### 2.1 创建 InputAction

1. 在 **Content Browser** 中导航到：`Content/RPG/Character/Player/Input/Action/`
2. 右键 → **Input** → **Input Action**
3. 命名为 `IA_EquipWeapon`

### 2.2 配置 InputAction

双击打开 `IA_EquipWeapon`，配置如下：
- **Trigger Type**: `Triggered`（按下时触发）
- 根据需要配置修饰键或其他触发条件

---

## 第三步：配置 InputConfig DataAsset

### 3.1 打开 InputConfig DataAsset

导航到：`Content/RPG/Character/Player/DA_InputConfig.uasset`

双击打开该 DataAsset。

### 3.2 配置输入映射

在 Details 面板中：

**DefaultMappingContext**:
- 设置为你使用的输入映射上下文（如 `IMC_RPG_Default`）

**NativeInputActions**（原生输入，如移动、视角）:
- 配置移动、视角等基础输入

**AbilityInputActions**（技能输入）:
添加新的输入配置项：

```
InputTag: InputTag.EquipSword
InputAction: IA_EquipWeapon
```

这个配置建立了 **InputAction → InputTag** 的映射关系。

### 3.3 创建武器专用的 InputMappingContext（可选）

如果需要为武器创建独立的输入映射：

1. 导航到：`Content/RPG/Character/Player/Input/`
2. 右键 → **Input** → **Input Mapping Context**
3. 命名为 `IMC_RPG_Sword`
4. 打开并添加武器相关的 InputAction

---

## 第四步：创建技能蓝图

### 4.1 创建 GameplayAbility 蓝图

1. 在 **Content Browser** 中导航到：`Content/RPG/Character/Player/Ability/Equip/`
2. 右键 → **Gameplay Ability** → 选择基类 `RPGPlayerGameplayAbility`
3. 命名为 `GA_Player_EquipSword`

### 4.2 配置技能基础属性

双击打开 `GA_Player_EquipSword`，在 Details 面板中配置：

**Ability Tags**:
- 添加：`Player.Ability.Equip.Sword`

**Activation Owned Tags**:
- 可选：添加技能激活时拥有的标签

**Activation Required Tags**:
- 可选：添加技能激活需要的前置标签

**Activation Blocked Tags**:
- 可选：添加会阻止技能激活的标签

### 4.3 实现技能逻辑

`RPGPlayerAbility_EquipSword` 的 C++ 实现已包含以下功能：

```cpp
// 主要流程：
1. ActivateAbility() - 技能激活
   ↓
2. 获取武器数据（从 CombatComponent）
   ↓
3. 播放装备蒙太奇动画
   ↓
4. 等待 GameplayEvent（动画通知触发）
   ↓
5. OnEquipGameplayEventReceived() - 接收到事件
   ↓
6. AttachWeaponToCharacter() - 附加武器到角色
   ↓
7. LinkWeaponAnimLayer() - 链接武器动画层
   ↓
8. AddWeaponInputMappingContext() - 添加武器输入映射
   ↓
9. GrantWeaponAbilities() - 授予武器技能
   ↓
10. 设置当前装备武器 Tag
```

**在蓝图中你需要配置：**
- **SocketNameToAttach**: 武器附加到角色的 Socket 名称（如 `WeaponSocket`）
- 确保武器的 `PlayerWeaponData` 已正确配置（见下一步）

---

## 第五步：配置武器数据

### 5.1 创建或打开武器蓝图

导航到：`Content/RPG/Character/Player/Weapon/Sword/`

打开 `BP_Weapon_Sword`（武器 Actor 蓝图）。

### 5.2 配置 PlayerWeaponData

在武器的 Details 面板中找到 **WeaponData** 分类，配置 `PlayerWeaponData`：

**WeaponAnimLayerToLink**:
- 设置武器的动画层接口（如 `RPG_Weapon_Sword`）
- 用于链接武器专属动画

**EquipWeaponMontage**:
- 设置装备动画蒙太奇（如 `AM_EquipSword`）
- 播放此动画时，通过 AnimNotify 发送 `Player.Event.Equip.Sword` 事件

**UnequipWeaponMontage**:
- 设置卸下武器动画（可选）

**WeaponInputMappingContext**:
- 设置为 `IMC_RPG_Sword`（武器专用输入映射）
- 装备武器时会自动添加此输入映射

**DefaultWeaponAbilities**（武器默认技能）:
这是武器技能的核心配置，添加技能集合：

```
[0] 
  InputTag: InputTag.LightAttack.Sword
  AbilityToGrant: GA_Player_LightAttack_Sword

[1]
  InputTag: InputTag.HeavyAttack.Sword
  AbilityToGrant: GA_Player_HeavyAttack_Sword
```

当玩家装备武器时，这些技能会自动授予玩家。

**WeaponBaseDamage**:
- 配置武器基础伤害（ScalableFloat，支持等级缩放）

**Combat Behavior**:
- `MaxComboCount`: 最大连击数（如 4）
- `AttackSpeedMultiplier`: 攻击速度倍率（如 1.0）
- `BaseDamageMultiplier`: 基础伤害倍率（如 1.0）

### 5.3 配置武器的 GameplayTag

在武器蓝图的 Details 面板中：
- 确保武器的标识 Tag 与代码中一致：`Player.Weapon.Sword`

---

## 第六步：配置 StartupData

### 6.1 创建或打开 PlayerStartupData

导航到：`Content/RPG/DataAsset/StartUp/Player/`

创建或打开玩家启动数据资产（如 `DA_PlayerStartUp_Default`）。

**创建方法**：
1. 右键 → **Miscellaneous** → **Data Asset**
2. 选择 `DataAsset_PlayerStartUpData`
3. 命名并保存

### 6.2 配置启动技能

打开 StartupData DataAsset，配置以下字段：

**ActiveOnGivenAbilities**（立即授予的主动技能）:
```
- GA_Player_EquipSword
- GA_Player_UnequipSword
- GA_Player_Roll
```

**ReactiveAbilities**（被动/响应式技能）:
```
- GA_Player_HitReact
- GA_Player_Death
```

**StartUpGameplayEffect**（启动时应用的效果）:
```
- GE_Player_InitialStats（初始属性）
```

**PlayerStartUpAbilitySet**（带输入绑定的技能集合）:
这是关键配置，建立 **InputTag → Ability** 的映射：

```
[0]
  InputTag: InputTag.EquipSword
  AbilityToGrant: GA_Player_EquipSword

[1]
  InputTag: InputTag.UnequipSword
  AbilityToGrant: GA_Player_UnequipSword

[2]
  InputTag: InputTag.Roll
  AbilityToGrant: GA_Player_Roll
```

### 6.3 将 StartupData 应用到 PlayerState

1. 导航到：`Content/RPG/Character/Player/`
2. 打开 `PS_PlayerState`（PlayerState 蓝图）
3. 在 Details 面板中找到 **RPG|Startup** 分类
4. 设置 `PlayerStartUpData` 为你刚配置的 DataAsset

---

## 第七步：验证与测试

### 7.1 检查输入配置

确保以下配置链完整：

```
键盘按键 → InputAction (IA_EquipWeapon) 
         → InputConfig DataAsset (DA_InputConfig)
         → InputTag (InputTag.EquipSword)
         → StartupData (PlayerStartUpAbilitySet)
         → Ability (GA_Player_EquipSword)
```

### 7.2 在编辑器中测试

1. 打开测试关卡
2. 放置玩家角色（`BP_RPGPlayerCharacter`）
3. **Play** 游戏
4. 按下配置的按键（如 E 键）触发装备技能

### 7.3 调试技巧

**启用日志输出**：
在 `RPGPlayerAbility_EquipSword.cpp` 中已包含详细日志：

```cpp
UE_LOG(LogRPGPlayerAbility_EquipSword, Log, TEXT("ActivateAbility %s"), *GetName())
```

打开 **Output Log**（Window → Developer Tools → Output Log）查看技能激活状态。

**常见问题排查**：

| 问题 | 可能原因 | 解决方案 |
|------|---------|---------|
| 技能未激活 | InputTag 未正确绑定 | 检查 InputConfig 和 StartupData 配置 |
| 武器未出现 | CombatComponent 未注册武器 | 确保武器已添加到 CombatComponent |
| 动画未播放 | 蒙太奇未配置 | 检查武器的 EquipWeaponMontage 字段 |
| 输入无响应 | InputMappingContext 未添加 | 检查 DefaultMappingContext 配置 |

---

## 完整配置示例

### 示例：配置装备剑技能

**1. GameplayTag（C++）**：
```cpp
UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipSword, "InputTag.EquipSword");
UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Sword, "Player.Ability.Equip.Sword");
UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Sword, "Player.Event.Equip.Sword");
UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Sword, "Player.Weapon.Sword");
```

**2. InputAction（蓝图）**：
- 文件：`Content/RPG/Character/Player/Input/Action/IA_EquipWeapon`
- Trigger Type: `Triggered`

**3. InputConfig DataAsset**：
- 文件：`Content/RPG/Character/Player/DA_InputConfig`
- AbilityInputActions:
  ```
  InputTag: InputTag.EquipSword
  InputAction: IA_EquipWeapon
  ```

**4. 技能蓝图**：
- 文件：`Content/RPG/Character/Player/Ability/Equip/GA_Player_EquipSword`
- 基类：`RPGPlayerGameplayAbility`
- SocketNameToAttach: `WeaponSocket`

**5. 武器蓝图**：
- 文件：`Content/RPG/Character/Player/Weapon/Sword/BP_Weapon_Sword`
- PlayerWeaponData:
  ```
  WeaponAnimLayerToLink: RPG_Weapon_Sword
  EquipWeaponMontage: AM_EquipSword
  WeaponInputMappingContext: IMC_RPG_Sword
  DefaultWeaponAbilities:
    - InputTag: InputTag.LightAttack.Sword
      AbilityToGrant: GA_Player_LightAttack_Sword
    - InputTag: InputTag.HeavyAttack.Sword
      AbilityToGrant: GA_Player_HeavyAttack_Sword
  ```

**6. StartupData**：
- 文件：`Content/RPG/DataAsset/StartUp/Player/DA_PlayerStartUp_Default`
- PlayerStartUpAbilitySet:
  ```
  InputTag: InputTag.EquipSword
  AbilityToGrant: GA_Player_EquipSword
  ```

**7. PlayerState**：
- 文件：`Content/RPG/Character/Player/PS_PlayerState`
- PlayerStartUpData: `DA_PlayerStartUp_Default`

---

## 关键代码解析

### RPGPlayerAbility_EquipSword 核心逻辑

**激活技能**（ActivateAbility）：
```cpp
void URPGPlayerAbility_EquipSword::ActivateAbility(...)
{
    // 1. 获取角色和控制器引用
    CachedPlayerCharacter = GetPlayerCharacterFromActorInfo();
    CachedPlayerController = GetPlayerControllerFromActorInfo();
    
    // 2. 获取战斗组件
    CombatComponent = CachedPlayerCharacter->GetPlayerCombatComponent();
    
    // 3. 获取剑武器
    CachedSwordWeapon = GetSwordWeapon(); // 通过 Player_Weapon_Sword Tag 获取
    
    // 4. 播放装备动画
    UAbilityTask_PlayMontageAndWait* MontageTask = ...;
    MontageTask->OnCompleted.AddDynamic(this, &OnEquipMontageCompleted);
    MontageTask->ReadyForActivation();
    
    // 5. 等待动画通知事件
    WaitForEquipGameplayEvent();
}
```

**接收装备事件**（OnEquipGameplayEventReceived）：
```cpp
void URPGPlayerAbility_EquipSword::OnEquipGameplayEventReceived(FGameplayEventData Payload)
{
    // 动画通知触发此事件 → 附加武器到骨骼
    AttachWeaponToCharacter();
}
```

**附加武器到角色**（AttachWeaponToCharacter）：
```cpp
void URPGPlayerAbility_EquipSword::AttachWeaponToCharacter()
{
    // 1. 将武器附加到角色骨骼的指定 Socket
    SwordWeapon->AttachToComponent(MeshComp, ..., SocketNameToAttach);
    
    // 2. 链接武器动画层
    LinkWeaponAnimLayer(SwordWeapon);
    
    // 3. 添加武器输入映射
    AddWeaponInputMappingContext(SwordWeapon);
    
    // 4. 授予武器技能
    GrantWeaponAbilities(SwordWeapon);
    
    // 5. 设置当前装备武器 Tag
    CombatComponent->CurrentEquippedWeaponTag = Player_Weapon_Sword;
}
```

**授予武器技能**（GrantWeaponAbilities）：
```cpp
void URPGPlayerAbility_EquipSword::GrantWeaponAbilities(ARPGPlayerWeapon* Weapon)
{
    // 从武器数据中获取技能集合
    const TArray<FRPGPlayerAbilitySet>& WeaponAbilities = 
        Weapon->PlayerWeaponData.DefaultWeaponAbilities;
    
    // 通过 ASC 授予技能
    ASC->GrantPlayerWeaponAbility(WeaponAbilities, GetAbilityLevel(), GrantedHandles);
    
    // 保存授予的技能句柄（用于后续移除）
    Weapon->AssignGrantedAbilitySpecHandles(GrantedHandles);
}
```

---

## 扩展：添加新武器技能

如果你想为新武器（如斧头）添加装备技能：

### 1. 定义新的 GameplayTag

```cpp
// RPGGameplayTags.h
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EquipAxe);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Equip_Axe);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Equip_Axe);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Weapon_Axe);

// RPGGameplayTags.cpp
UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipAxe, "InputTag.EquipAxe");
UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Axe, "Player.Ability.Equip.Axe");
UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Axe, "Player.Event.Equip.Axe");
UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Axe, "Player.Weapon.Axe");
```

### 2. 创建装备技能

- 复制 `GA_Player_EquipSword` → `GA_Player_EquipAxe`
- 修改引用的 Tag 和武器数据

### 3. 创建武器蓝图

- 创建 `BP_Weapon_Axe`
- 配置 `PlayerWeaponData`

### 4. 更新 StartupData

在 `DA_PlayerStartUp_Default` 中添加：
```
InputTag: InputTag.EquipAxe
AbilityToGrant: GA_Player_EquipAxe
```

---

## 注意事项

### ⚠️ 重要提醒

1. **Tag 命名一致性**：确保 C++ 代码、蓝图配置中的 Tag 字符串完全一致
2. **执行顺序**：
   - PlayerState BeginPlay → 初始化 ASC → 应用 StartupData → 授予技能
   - 确保 ASC 的 `InitAbilityActorInfo` 在授予技能前调用
3. **网络复制**：技能授予会自动复制到客户端，但输入映射需要单独处理
4. **武器注册**：武器必须在 CombatComponent 中注册后才能被装备技能获取

### ✅ 最佳实践

1. **模块化设计**：每个武器独立的 InputMappingContext 和技能集合
2. **数据驱动**：所有配置通过 DataAsset 管理，便于策划调整
3. **复用基类**：所有玩家技能继承 `RPGPlayerGameplayAbility`
4. **日志完善**：在关键节点添加 UE_LOG 便于调试
5. **版本控制**：DataAsset 易于版本管理和协作

---

## 相关文件索引

### C++ 代码

| 文件 | 路径 | 说明 |
|------|------|------|
| RPGGameplayTags.h | `Source/RPG/Public/RPGGameplayTags.h` | GameplayTag 声明 |
| RPGGameplayTags.cpp | `Source/RPG/Private/RPGGameplayTags.cpp` | GameplayTag 定义 |
| RPGPlayerAbility_EquipSword.h | `Source/RPG/Public/AbilitySystem/Abilities/Player/RPGPlayerAbility_EquipSword.h` | 装备技能头文件 |
| RPGPlayerAbility_EquipSword.cpp | `Source/RPG/Private/AbilitySystem/Abilities/Player/RPGPlayerAbility_EquipSword.cpp` | 装备技能实现 |
| RPGPlayerGameplayAbility.h | `Source/RPG/Public/AbilitySystem/Abilities/RPGPlayerGameplayAbility.h` | 玩家技能基类 |
| DataAsset_InputConfig.h | `Source/RPG/Public/DataAsset/Input/DataAsset_InputConfig.h` | 输入配置 DataAsset |
| DataAsset_PlayerStartUpData.h | `Source/RPG/Public/DataAsset/StartUpDate/DataAsset_PlayerStartUpData.h` | 玩家启动数据 |
| RPGStructTypes.h | `Source/RPG/Public/Types/RPGStructTypes.h` | 结构体定义（FRPGPlayerAbilitySet 等） |
| RPGPlayerWeapon.h | `Source/RPG/Public/Items/Weapon/RPGPlayerWeapon.h` | 武器类 |
| RPGAbilitySystemComponent.h | `Source/RPG/Public/AbilitySystem/RPGAbilitySystemComponent.h` | ASC 组件 |

### 蓝图资源

| 资源 | 路径 | 说明 |
|------|------|------|
| GA_Player_EquipSword | `Content/RPG/Character/Player/Ability/Equip/GA_Player_EquipSword` | 装备技能蓝图 |
| IA_EquipWeapon | `Content/RPG/Character/Player/Input/Action/IA_EquipWeapon` | 装备输入动作 |
| DA_InputConfig | `Content/RPG/Character/Player/DA_InputConfig` | 输入配置 DataAsset |
| BP_Weapon_Sword | `Content/RPG/Character/Player/Weapon/Sword/BP_Weapon_Sword` | 剑武器蓝图 |
| IMC_RPG_Sword | `Content/RPG/Character/Player/Input/IMC_RPG_Sword` | 剑输入映射上下文 |
| DA_PlayerStartUp_Default | `Content/RPG/DataAsset/StartUp/Player/` | 玩家启动数据 |
| PS_PlayerState | `Content/RPG/Character/Player/PS_PlayerState` | 玩家 PlayerState |

---

## 参考文档

- [ASC混合架构设计](./ASC混合架构设计.md)
- [StartupData使用指南](./StartupData使用指南.md)
- [Unreal Engine GAS 官方文档](https://docs.unrealengine.com/en-US/GameplayAbilitySystem/)
- [Enhanced Input 官方文档](https://docs.unrealengine.com/en-US/interactive-experiences/input/enhanced-input/unreal-engine-enhanced-input-system/)

---

**文档版本**: 1.0  
**最后更新**: 2026-04-11  
**维护者**: RPG开发团队
