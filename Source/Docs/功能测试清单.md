# RPG 项目功能测试清单

## 概述

本清单基于当前项目已实现的 C++ 代码和蓝图资产整理，用于系统性验证各模块功能。每个测试项包含 **前置配置**、**测试步骤** 和 **预期结果**。

---

## 一、GameMode 与基础框架

### 1.1 GameMode 配置验证

| 项目 | 说明 |
|------|------|
| C++ 类 | `ARPGGameModeBase` (继承 `AGameModeBase`) |
| 蓝图 | `BP_RPGGameModeBase` (`Content/RPG/GameMode/`) |

**前置配置：**
- 在 `World Settings` → `Game Mode Override` 中设置为 `BP_RPGGameModeBase`
- 确认 `BP_RPGGameModeBase` 中的 `Default Pawn Class` = `BP_RPGPlayerCharacter`
- 确认 `Player Controller Class` = `BP_RPGPlayerController`
- 确认 `Player State Class` = `ARPGPlayerState`（C++ 构造函数已默认设置）

**测试步骤：**
1. PIE 启动游戏
2. 检查是否正确生成玩家角色

**预期结果：**
- [x] 游戏正常启动，生成 `BP_RPGPlayerCharacter` 实例
- [x] PlayerController 类型为 `BP_RPGPlayerController`
- [x] PlayerState 类型为 `ARPGPlayerState`

---

## 二、玩家角色系统

### 2.1 角色生成与相机

| 项目 | 说明 |
|------|------|
| C++ 类 | `ARPGPlayerCharacter` → `ABaseCharacter` → `ACharacter` |
| 蓝图 | `BP_RPGPlayerCharacter` (`Content/RPG/Character/Player/`) |
| 骨骼网格体 | `SK_RPG_Mannequin` (`Content/RPG/Character/Player/Mesh/`) |

**测试步骤：**
1. PIE 启动，观察角色是否正确显示
2. 检查相机弹簧臂（`CameraBoom`）和跟随相机（`FollowCamera`）是否正常工作

**预期结果：**
- [ ] 角色正确加载骨骼网格体，站立在场景中
- [ ] 第三人称相机跟随角色，弹簧臂工作正常

### 2.2 角色配置数据资产

| 项目 | 说明 |
|------|------|
| C++ 类 | `UDataAsset_CharacterConfig` |
| 蓝图资产 | `DA_Swordsman` (`Content/RPG/DataAsset/CharacterConfig/`) |
| 配置字段 | `CharacterName`、`CharacterClass`（职业枚举）、`CharacterDescription` |

**前置配置：**
- 打开 `BP_RPGPlayerCharacter` → Details → `CharacterData` → 设置 `CharacterConfig` = `DA_Swordsman`
- 打开 `DA_Swordsman`，确认已配置：
  - `CharacterName`（如 "Swordsman"）
  - `CharacterClass`（如 `战士`）
  - `CharacterDescription`

**测试步骤：**
1. 运行时通过蓝图/调试命令调用 `GetCharacterConfig()` 检查返回值

**预期结果：**
- [ ] `GetCharacterConfig()` 返回有效的 `DA_Swordsman` 数据
- [ ] 角色名、职业、描述与配置一致

---

## 三、输入系统

### 3.1 基础移动与视角输入

| 项目 | 说明 |
|------|------|
| C++ 类 | `URPGEnhancedInputComponent`、`UDataAsset_InputConfig` |
| 蓝图资产 | `DA_InputConfig` (`Content/RPG/Character/Player/`)、`IMC_RPG_Default` (`Content/RPG/Input/`) |
| Input Actions | `IA_Default_Move`、`IA_Default_Look` (`Content/RPG/Input/Action/`) |
| Gameplay Tags | `InputTag.Move`、`InputTag.Look` |

**前置配置：**
- 打开 `DA_InputConfig`，确认：
  - `DefaultMappingContext` = `IMC_RPG_Default`
  - `NativeInputActions` 中配置了 `InputTag.Move` → `IA_Default_Move`、`InputTag.Look` → `IA_Default_Look`
- 打开 `BP_RPGPlayerCharacter` → `CharacterData` → `InputConfigDataAsset` = `DA_InputConfig`

**测试步骤：**
1. WASD 移动
2. 鼠标移动视角

**预期结果：**
- [ ] WASD 可以控制角色前后左右移动
- [ ] 鼠标可以旋转视角/相机

### 3.2 技能输入绑定

| 项目 | 说明 |
|------|------|
| 绑定方法 | `URPGEnhancedInputComponent::BindAbilityInputAction` |
| 传递路径 | Input → `RPGAbilitySystemComponent::OnAbilityInputPressed/Released` |

**前置配置：**
- 在 `DA_InputConfig` 的 `AbilityInputActions` 中添加技能输入映射（如轻攻击、重攻击的 InputTag）
- 确保对应的 `InputAction` 资产已创建

**测试步骤：**
1. 按下配置的技能键
2. 观察是否触发 ASC 的 `OnAbilityInputPressed`

**预期结果：**
- [ ] 按键正确触发对应的 InputTag 事件
- [ ] 松开按键触发 `OnAbilityInputReleased`

---

## 四、平滑转向系统

### 4.1 基于速度和角度的动态转向

| 项目 | 说明 |
|------|------|
| 核心函数 | `SmoothRotateToTarget`、`CalculateDynamicTurnSpeed` |
| 配置参数 | `BaseTurnSpeed`、`MaxTurnSpeed`、`SpeedTurnMultiplier`、`AngleTurnMultiplier` |
| 调试开关 | `bShowRotationDebug`（绿色=当前朝向，红色=目标朝向） |

**前置配置：**
- 在 `BP_RPGPlayerCharacter` → `Movement|Rotation` 分类中调整：
  - `BaseTurnSpeed`（建议 300-500）
  - `MaxTurnSpeed`（建议 600-1000）
  - `SpeedTurnMultiplier`（建议 0.5-1.0）
  - `AngleTurnMultiplier`（建议 0.3-0.8）

**测试步骤：**
1. 开启 `bShowRotationDebug = true`（在 `Debug` 分类下）
2. 原地站立后突然按方向键，观察转向速度
3. 全速奔跑中变换方向，观察转向速度变化
4. 180度大角度转向，观察是否加速转向

**预期结果：**
- [ ] 静止时转向使用 `BaseTurnSpeed`，转向较慢但平滑
- [ ] 高速移动时转向加快（受 `SpeedTurnMultiplier` 影响）
- [ ] 大角度偏转时转向速度额外增加（受 `AngleTurnMultiplier` 影响）
- [ ] 调试射线正确显示：绿色线=当前朝向，红色线=目标朝向
- [ ] 转向过程平滑，无突然跳转

---

## 五、GAS 能力系统（ASC 混合架构）

> 详细配置参考：[ASC混合架构设计](./ASC混合架构设计.md)

### 5.1 玩家 ASC 初始化（PlayerState 架构）

| 项目 | 说明 |
|------|------|
| ASC 所在位置 | `ARPGPlayerState` |
| 组件 | `URPGAbilitySystemComponent` + `URPGAttributeSet` |
| 初始化 | `InitAbilityActorInfo(PlayerState, PlayerCharacter)` |

**测试步骤：**
1. PIE 启动后，在蓝图中对玩家角色调用 `Get Player State` → `Get RPG Ability System Component`
2. 或使用 `URPGFunctionLibrary::NativeGetRPGASCFromActor(PlayerCharacter)`

**预期结果：**
- [ ] ASC 非空，成功从 PlayerState 获取
- [ ] ASC 的 Avatar Actor = PlayerCharacter
- [ ] ASC 的 Owner Actor = PlayerState

### 5.2 玩家属性集 (AttributeSet)

| 属性 | 类型 | 说明 |
|------|------|------|
| `CurrentHealth` | `FGameplayAttributeData` | 当前生命值 |
| `MaxHealth` | `FGameplayAttributeData` | 最大生命值 |
| `CurrentRage` | `FGameplayAttributeData` | 当前怒气值 |
| `MaxRage` | `FGameplayAttributeData` | 最大怒气值 |
| `AttackPower` | `FGameplayAttributeData` | 攻击力 |
| `DefensePower` | `FGameplayAttributeData` | 防御力 |
| `DamageTaken` | `FGameplayAttributeData` | 受到的伤害（Meta属性） |

**测试步骤：**
1. 运行时通过蓝图节点读取 `Get RPG Attribute Set` → 查看各属性值
2. 应用一个修改 `CurrentHealth` 的 GameplayEffect，观察 `PostGameplayEffectExecute` 是否被调用

**预期结果：**
- [ ] 属性初始值正确（需配合 StartupData 中的 GameplayEffect 初始化）
- [ ] 修改属性后回调正确触发
- [ ] `DamageTaken` 作为 Meta 属性正确计算并应用到 `CurrentHealth`

### 5.3 敌人 ASC 初始化（Character 架构）

| 项目 | 说明 |
|------|------|
| C++ 类 | `AEnemyCharacter` (继承 `ABaseCharacter`，实现 `IAbilitySystemInterface`) |
| ASC 所在位置 | 直接在 `AEnemyCharacter` 上 |
| 初始化 | `InitAbilityActorInfo(this, this)` |

**前置配置：**
- 创建继承自 `AEnemyCharacter` 的蓝图
- 在蓝图 Details → `RPG|Startup` → 设置 `EnemyStartUpData`

**测试步骤：**
1. 在场景中放置敌人蓝图
2. 运行后对敌人调用 `GetRPGAbilitySystemComponent()`

**预期结果：**
- [ ] 敌人 ASC 非空，直接从 Character 获取
- [ ] 敌人 AttributeSet 非空
- [ ] `IAbilitySystemInterface` 接口正确实现

### 5.4 统一 ASC 访问（FunctionLibrary）

| 函数 | 说明 |
|------|------|
| `NativeGetRPGASCFromActor` | 自动判断玩家/敌人，统一获取 ASC |
| `AddGameplayTagToActorIfNone` | 给 Actor 添加 Tag（如果没有） |
| `RemoveGameplayFromActorIfFound` | 移除 Actor 的 Tag |
| `BP_DoesActorHasTag` | 检查 Actor 是否有指定 Tag |
| `NativeGetPawnCombatComponentFromActor` | 获取战斗组件 |
| `IsTargetPawnHostile` | 判断目标是否敌对 |
| `ComputeHitReactDirectionTag` | 计算受击方向 Tag |
| `GetScalableFloatValueAtLevel` | 获取等级缩放数值 |

**测试步骤：**
1. 对玩家角色调用 `NativeGetRPGASCFromActor` → 应从 PlayerState 获取
2. 对敌人角色调用 `NativeGetRPGASCFromActor` → 应从 Character 直接获取
3. 测试 Tag 的添加/移除/查询
4. 测试 `IsTargetPawnHostile`（玩家 vs 敌人）
5. 测试 `ComputeHitReactDirectionTag`（从不同方向攻击敌人）

**预期结果：**
- [ ] 玩家和敌人都能正确获取 ASC
- [ ] Tag 操作正常（添加、移除、查询）
- [ ] 敌对判断正确（玩家和敌人互为敌对）
- [ ] 受击方向计算正确（前/后/左/右）

---

## 六、StartupData 初始化系统

> 详细配置参考：[StartupData使用指南](./StartupData使用指南.md)

### 6.1 玩家 StartupData

| 项目 | 说明 |
|------|------|
| C++ 类 | `UDataAsset_PlayerStartUpData` → `UDataAsset_StartUpDataBase` |
| 配置位置 | `ARPGPlayerState` → `RPG|Startup` → `PlayerStartUpData` |
| 功能 | 授予初始能力 + GameplayEffect + 带 InputTag 的能力集 |

**前置配置：**
1. 创建 `DataAsset_PlayerStartUpData` 资产（如 `DA_PlayerStartUp_Default`）
2. 配置 `ActiveOnGivenAbilities`（自动激活的能力）
3. 配置 `ReactiveAbilities`（被动/响应式能力）
4. 配置 `StartUpGameplayEffect`（初始属性效果，如设置 MaxHealth=100）
5. 配置 `PlayerStartUpAbilitySet`（带 InputTag 的能力，如攻击技能）
6. 在 PlayerState 蓝图 → `RPG|Startup` 中设置该 DataAsset

**测试步骤：**
1. 配置好 StartupData 后 PIE 启动
2. 检查初始属性是否正确设置（如 MaxHealth）
3. 检查技能是否已授予
4. 按下技能对应的按键，验证是否可以激活

**预期结果：**
- [ ] `BeginPlay` 后自动调用 `InitializeStartupData`
- [ ] `ActiveOnGivenAbilities` 中的能力被授予并自动激活
- [ ] `ReactiveAbilities` 中的能力被授予（等待触发）
- [ ] `StartUpGameplayEffect` 正确应用（属性初始化）
- [ ] `PlayerStartUpAbilitySet` 中的能力绑定到对应的 InputTag

### 6.2 敌人 StartupData

| 项目 | 说明 |
|------|------|
| C++ 类 | `UDataAsset_EnemyStartUpData` → `UDataAsset_StartUpDataBase` |
| 配置位置 | `AEnemyCharacter` 蓝图 → `RPG|Startup` → `EnemyStartUpData` |

**前置配置：**
1. 创建 `DataAsset_EnemyStartUpData` 资产
2. 配置基础能力和效果
3. 在敌人蓝图中设置该 DataAsset

**测试步骤：**
1. 放置敌人到场景，PIE 启动
2. 检查敌人是否被授予了能力和初始属性

**预期结果：**
- [ ] 敌人生成后自动应用 StartupData
- [ ] 能力和效果正确授予

---

## 七、动画系统

### 7.1 基础动画实例

| 项目 | 说明 |
|------|------|
| C++ 类 | `URPGBaseAnimInstance` → `URPGCharacterAnimInstance` |
| 蓝图 | `ABP_Mannequin_Base` (`Content/RPG/Character/Player/Mannequin/Animations/`) |
| BlendSpace | `BS_UnarmedLocomotion`、`BS_UnarmedLocomotion_8Direction` |

**前置配置：**
- `BP_RPGPlayerCharacter` 的 Mesh 组件 → `Anim Class` = `ABP_Mannequin_Base`

**测试步骤：**
1. 观察角色站立 idle 动画
2. WASD 移动，观察移动动画混合
3. 不同速度下观察步态变化

**预期结果：**
- [ ] `GroundSpeed`、`Direction`、`Velocity` 等参数实时更新
- [ ] `bIsMoving`、`bIsFalling`、`bIsGrounded` 状态正确
- [ ] `bIsIdle` / `bIsWalking` / `bIsRunning` / `bIsSprinting` 根据速度阈值正确切换
- [ ] `GaitAmount` 步态比例平滑变化
- [ ] BlendSpace 混合过渡流畅

### 7.2 Linked Anim Layers（武器动画层切换）

| 项目 | 说明 |
|------|------|
| C++ 类 | `URPGItemAnimLayersBase` (Abstract) |
| 蓝图 | `ABP_RPGItemAnimLayersBase`、`ALI_ItemAnimLayers`（Anim Layer Interface） |
| 功能 | 运行时切换武器动画层 |

**前置配置：**
- 在武器数据（`FRPGPlayerWeaponData`）中设置 `WeaponAnimLayerToLink`
- `ABP_Mannequin_Base` 需实现 `ALI_ItemAnimLayers` 接口

**测试步骤：**
1. 装备武器后，观察动画层是否正确链接
2. 切换武器类型，观察动画层是否切换
3. 检查 `URPGItemAnimLayersBase` 从 `PlayerCombatComponent` 同步的数据

**预期结果：**
- [ ] `LinkAnimLayer` 正确链接新动画层
- [ ] `UnlinkAnimLayer` 正确卸载动画层
- [ ] `CombatState`、`ComboIndex`、`MaxComboCount`、`bIsInComboWindow`、`AttackSpeedMultiplier` 每帧同步
- [ ] 动画层切换无明显卡顿

---

## 八、武器与装备系统

### 8.1 武器基础类

| 项目 | 说明 |
|------|------|
| C++ 类层次 | `ARPGWeaponBase` → `ARPGPlayerWeapon` → `APlayerWeapon_Sword` |
| 蓝图 | `BP_Weapon_Sword` (`Content/RPG/Item/Weapon/Sword/`) |
| 静态网格体 | `SM_Weapon_Sword`、`RPG_Weapon_Sword` |
| 碰撞组件 | `UBoxComponent` (WeaponCollisionBox) |

**前置配置：**
- 打开 `BP_Weapon_Sword`，确认：
  - `WeaponMesh` 设置了正确的静态网格体
  - `WeaponCollisionBox` 正确配置碰撞范围
  - `PlayerWeaponData` 已配置（见下方 8.2）

**测试步骤：**
1. 游戏中生成/装备武器
2. 检查武器是否正确附着到角色骨骼上
3. 检查碰撞盒初始状态（应为关闭）

**预期结果：**
- [ ] 武器正确显示在角色手中
- [ ] 碰撞盒默认禁用
- [ ] `OnCollisionBoxBeginOverlap` / `OnCollisionBoxEndOverlap` 绑定正确

### 8.2 武器数据配置 (FRPGPlayerWeaponData)

在 `BP_Weapon_Sword` → `WeaponData` 分类中配置：

| 字段 | 说明 | 示例值 |
|------|------|--------|
| `WeaponAnimLayerToLink` | 武器动画层类 | `ABP_RPGItemAnimLayersBase` 子类 |
| `WeaponInputMappingContext` | 武器专用输入映射 | 武器专用 IMC |
| `DefaultWeaponAbilities` | 武器自带技能集 | 轻攻击/重攻击 Ability + InputTag |
| `WeaponBaseDamage` | 基础伤害（ScalableFloat） | 根据等级曲线 |
| `SoftWeaponIconTexture` | 武器图标 | 图标纹理 |
| `MaxComboCount` | 最大连击数 | 4 |
| `AttackSpeedMultiplier` | 攻击速度倍率 | 1.0 |
| `BaseDamageMultiplier` | 基础伤害倍率 | 1.0 |

### 8.3 装备武器流程

| 项目 | 说明 |
|------|------|
| 入口函数 | `ARPGPlayerCharacter::EquipWeapon(ERPGWeaponType)` |
| 武器类型 | `None`、`Sword1H`、`Sword2H`、`Bow`、`Staff`、`DualBlade`、`Spear` |
| 查询 | `GetCurrentWeaponType()` |

**测试步骤：**
1. 调用 `EquipWeapon(Sword1H)` 装备单手剑
2. 检查 `GetCurrentWeaponType()` 返回值
3. 观察动画层是否切换到剑术动画

**预期结果：**
- [ ] `CurrentWeaponType` 正确更新
- [ ] 动画层正确切换
- [ ] 武器正确显示

---

## 九、战斗系统

### 9.1 战斗组件

| 项目 | 说明 |
|------|------|
| 基类 | `UPawnCombatComponent` (继承 `UPawnExtensionComponentBase`) |
| 玩家子类 | `UPlayerCombatComponent` |
| 接口 | `IPawnCombatInterface` |

**武器管理功能测试：**

| 函数 | 测试点 |
|------|--------|
| `RegisterSpawnWeapon` | 注册武器到 Tag Map，可选设为当前装备 |
| `GetCharacterCarriedWeaponByTag` | 通过 Tag 获取已注册武器 |
| `GetCharacterCurrentEquippedWeapon` | 获取当前装备的武器 |
| `ToggleWeaponCollision` | 开关武器碰撞（支持当前装备/左手/右手） |

**测试步骤：**
1. 注册武器：`RegisterSpawnWeapon(WeaponTag, WeaponInstance, true)`
2. 查询武器：`GetCharacterCurrentEquippedWeapon()` 应返回注册的武器
3. 开启碰撞：`ToggleWeaponCollision(true)` → 攻击时碰撞检测应生效
4. 关闭碰撞：`ToggleWeaponCollision(false)`

**预期结果：**
- [ ] 武器正确注册到 `CharacterCarriedWeaponMap`
- [ ] `CurrentEquippedWeaponTag` 正确设置
- [ ] 碰撞开关正常工作
- [ ] `OnHitTargetActor` / `OnWeaponPullerFromTargetActor` 回调正确触发

### 9.2 连招系统

| 函数 | 说明 |
|------|------|
| `StartAttack` | 开始第一次攻击 |
| `TryComboAttack` | 尝试衔接下一段连招 |
| `ResetCombo` | 重置连招状态 |
| `OpenComboWindow` / `CloseComboWindow` | 打开/关闭连招输入窗口 |

**测试步骤：**
1. 按攻击键 → 调用 `StartAttack`
2. 在连招窗口内再次按攻击键 → `TryComboAttack`
3. 连续按攻击键，测试连招到 `MaxComboCount`
4. 连招窗口关闭后按攻击键，应无法衔接
5. 完整连招结束后，`CombatState` 应回到 `Idle`

**预期结果：**
- [ ] `ComboIndex` 从 0 递增到 `MaxComboCount - 1`
- [ ] `CombatState` 在 `Idle` → `Attacking` 之间正确切换
- [ ] `bIsInComboWindow` 在连招窗口期正确为 `true`
- [ ] 超过 `MaxComboCount` 后不再接受连招输入
- [ ] `ResetCombo` 正确重置所有状态
- [ ] `CanAttack()` 在合适时机返回 `true`/`false`
- [ ] `IsAttacking()` 正确反映攻击状态

### 9.3 武器碰撞与伤害

| 项目 | 说明 |
|------|------|
| 碰撞委托 | `OnWeaponHitTarget`、`OnWeaponPulledFromTarget` |
| 伤害计算 | `GetPlayerCurrentEquippedWeaponDamageAtLevel` |
| 命中判断 | `OverlappedActors` 数组防止重复命中 |

**测试步骤：**
1. 攻击敌人，观察 `OnHitTargetActor` 是否触发
2. 检查同一次攻击不会对同一目标重复伤害
3. 检查 `ComputeHitReactDirectionTag` 返回正确的受击方向

**预期结果：**
- [ ] 武器碰撞正确检测到敌人
- [ ] 单次攻击对同一敌人只触发一次命中
- [ ] 伤害数值正确（基于武器 `WeaponBaseDamage` 和等级）
- [ ] 受击方向计算正确

---

## 十、GAS 技能能力

### 10.1 基础 GameplayAbility

| 项目 | 说明 |
|------|------|
| 基类 | `URPGGameplayAbility` → `UGameplayAbility` |
| 玩家子类 | `URPGPlayerGameplayAbility` |
| 激活策略 | `ERPGAbilityActivationPolicy`（`OnTriggered` / `OnGive`） |

**测试步骤：**
1. 创建继承 `URPGPlayerGameplayAbility` 的蓝图技能
2. 在蓝图中调用 `GetPlayerCharacterFromActorInfo()` 获取玩家角色
3. 在蓝图中调用 `GetPlayerControllerFromActorInfo()` 获取控制器
4. 在蓝图中调用 `GetRPGAbilitySystemComponentFromActorInfo()` 获取 ASC
5. 测试 `MakePlayerDamageEffectSpecHandle` 创建伤害效果

**预期结果：**
- [ ] 能力可以通过 InputTag 激活
- [ ] Actor Info 正确返回玩家角色和控制器
- [ ] 伤害 EffectSpec 正确创建（包含 WeaponBaseDamage、AttackTag、ComboCount）

### 10.2 武器能力管理

| 函数 | 说明 |
|------|------|
| `GrantPlayerWeaponAbility` | 授予武器专属技能并返回 SpecHandles |
| `RemovedGrantPlayerWeaponAbility` | 通过 SpecHandles 移除已授予的武器技能 |
| `TryActivateAbilityByTag` | 通过 Tag 激活技能 |

**测试步骤：**
1. 装备武器后检查 `DefaultWeaponAbilities` 是否被授予
2. 切换武器后检查旧武器的技能是否被移除、新武器技能是否被授予
3. 使用 `TryActivateAbilityByTag` 手动激活特定技能

**预期结果：**
- [ ] 武器技能正确授予，`GrantAbilitySpecHandles` 记录了句柄
- [ ] 武器卸下时技能正确移除
- [ ] `TryActivateAbilityByTag` 能按 Tag 正确激活技能

---

## 十一、Gameplay Tags

### 11.1 已注册的 Native Tags

| Tag | 用途 |
|------|------|
| `InputTag.Move` | 移动输入 |
| `InputTag.Look` | 视角输入 |
| `Player.Event.HitPause` | 玩家命中顿帧事件 |
| `Shared.Event.MeleeHit` | 近战命中事件 |
| `Shared.SetByCaller.BaseDamage` | SetByCaller 基础伤害 |
| `Shared.Status.Dead` | 死亡状态 |

**测试步骤：**
1. 确认以上 Tag 在 `Project Settings → Gameplay Tags` 中存在
2. 测试 `AddGameplayTagToActorIfNone` 添加 `Shared.Status.Dead`
3. 测试 `BP_DoesActorHasTag` 查询 Tag
4. 测试 `RemoveGameplayFromActorIfFound` 移除 Tag

**预期结果：**
- [ ] 所有 Native Tag 正确注册
- [ ] Tag 的增删查操作正常

---

## 十二、蓝图资产完整性检查

### 12.1 资产清单

| 路径 | 资产名 | 类型 | 必须配置项 |
|------|--------|------|------------|
| `Content/RPG/GameMode/` | `BP_RPGGameModeBase` | GameMode | Default Pawn, PlayerController, PlayerState |
| `Content/RPG/Controller/` | `BP_RPGPlayerController` | PlayerController | - |
| `Content/RPG/Character/Player/` | `BP_RPGPlayerCharacter` | PlayerCharacter | InputConfig, CharacterConfig, AnimClass |
| `Content/RPG/Character/Player/` | `DA_InputConfig` | InputConfig DataAsset | DefaultMappingContext, NativeInputActions |
| `Content/RPG/DataAsset/CharacterConfig/` | `DA_Swordsman` | CharacterConfig | CharacterName, CharacterClass |
| `Content/RPG/Input/` | `IMC_RPG_Default` | InputMappingContext | Move/Look 按键绑定 |
| `Content/RPG/Input/Action/` | `IA_Default_Move` | InputAction | Axis2D |
| `Content/RPG/Input/Action/` | `IA_Default_Look` | InputAction | Axis2D |
| `Content/RPG/Item/Weapon/Sword/` | `BP_Weapon_Sword` | PlayerWeapon | PlayerWeaponData |
| `Content/RPG/Item/Weapon/Sword/` | `SM_Weapon_Sword` | StaticMesh | - |
| `Content/RPG/Character/Player/Mannequin/Animations/` | `ABP_Mannequin_Base` | AnimBlueprint | AnimClass = URPGCharacterAnimInstance |
| `Content/RPG/Character/Player/Mannequin/Animations/LinkedLayers/` | `ABP_RPGItemAnimLayersBase` | AnimBlueprint | 武器动画层 |
| `Content/RPG/Character/Player/Mannequin/Animations/LinkedLayers/` | `ALI_ItemAnimLayers` | AnimLayerInterface | 定义动画层接口 |
| `Content/RPG/Character/Player/Mannequin/Animations/BlendSpace/` | `BS_UnarmedLocomotion` | BlendSpace | 移动混合空间 |
| `Content/RPG/Character/Player/Mannequin/Animations/BlendSpace/` | `BS_UnarmedLocomotion_8Direction` | BlendSpace | 8方向移动混合空间 |

### 12.2 逐项检查

**测试步骤：**
逐个打开上述蓝图资产，确认：

- [ ] 所有资产可以正常打开，无编译错误
- [ ] 所有必须配置项已正确设置（非 None）
- [ ] `BP_RPGPlayerCharacter` 编译无警告
- [ ] `ABP_Mannequin_Base` 编译无警告
- [ ] `BP_Weapon_Sword` 编译无警告

---

## 快速测试流程（冒烟测试）

以下是最小化测试步骤，可以快速验证核心功能：

1. **启动游戏** → 角色正常生成，相机正常 ✅/❌
2. **WASD移动** → 角色移动，动画正确播放 ✅/❌
3. **鼠标视角** → 相机跟随旋转 ✅/❌
4. **转向表现** → 角色平滑转向，无突然跳转 ✅/❌
5. **装备武器** → 武器正确显示，动画层切换 ✅/❌
6. **攻击连招** → 可以连续攻击，连招衔接 ✅/❌
7. **碰撞检测** → 攻击敌人触发命中 ✅/❌
8. **属性查看** → 生命/怒气/攻击力等属性有值 ✅/❌
9. **Tag查询** → GameplayTag 增删查正常 ✅/❌

---

## 已知待实现 / 注释掉的功能

以下代码已存在但被注释掉，后续启用时需要测试：

| 位置 | 注释内容 | 说明 |
|------|----------|------|
| `URPGGameplayAbility` | `ERPGAbilityActivationPolicy` | 能力激活策略配置 |
| `URPGGameplayAbility` | `GetPawnCombatComponentFromActorInfo` | 从 ActorInfo 获取战斗组件 |
| `URPGGameplayAbility` | `NativeApplyEffectSpecHandleToTarget` | 对目标应用效果 |
| `URPGGameplayAbility` | `BP_ApplyEffectSpecHandleToTarget` | 蓝图版应用效果（带成功/失败输出） |
| `URPGPlayerGameplayAbility` | `GetPlayerCombatComponentFromActorInfo` | 获取玩家战斗组件 |
| `URPGAttributeSet` | `CachedPawnUIInterface` | UI 接口缓存（用于属性变化 UI 通知） |
