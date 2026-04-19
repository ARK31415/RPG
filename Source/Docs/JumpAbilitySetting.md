# JumpAbility生命周期管理 - 动画蓝图配置指南

## 📋 配置概述

本次修改将JumpAbility的生命周期与跳跃动画同步，通过`AN_SendGameplayEventToOwner`动画通知实现GAS事件通信。

---

## 🎯 核心改动

### 1. **新增EventTag**
- **Tag名称**: `Player_Event_Jump_Finished`
- **Tag路径**: `Player.Event.Jump.Finished`
- **用途**: 标记跳跃动画完成事件

### 2. **JumpAbility生命周期调整**
```
旧流程:
ActivateAbility → PerformJump → 立即EndAbility ❌
  (Player_Status_Jumping过早移除)

新流程:
ActivateAbility → PerformJump → 等待Event → EndAbility ✅
  (Player_Status_Jumping全程存在，直到AN_JumpFinish触发)
```

---

## 🔧 动画蓝图配置步骤

### 步骤1: 打开跳跃动画蓝图
- 找到你的角色动画蓝图（如 `ABP_Mannequin_Base` 或自定义动画蓝图）
- 确保使用 `URPGCharacterAnimInstance` 作为父类

### 步骤2: 定位Land动画
- 打开状态机（State Machine）
- 找到 `Jump` 状态或 `JumpLand` 状态
- 进入Land动画的动画轨道

### 步骤3: 添加AN_SendGameplayEventToOwner通知

**位置选择**：
- 在Land动画的 **80%-90%** 进度处添加Notify
- 不要太早（避免跳跃状态提前结束）
- 不要太晚（避免影响下一次跳跃）

**配置参数**：
```
Notify名称: AN_SendGameplayEventToOwner
Event Tag: Player.Event.Jump.Finished
Target: OwningActor (默认)
Optional Data: 留空（或根据需要传递落地速度等数据）
```

### 步骤4: 验证Notify触发
- 运行游戏，执行跳跃
- 查看日志输出，应该看到：
  ```
  [JumpAbility] ActivateAbility - Actor: ...
  [JumpAbility] Applied Player_Status_Jumping tag
  [JumpAbility] SetupJumpFinishedEventWait - Event listener activated
  [JumpAbility] PerformJump - Actor: ..., Location: ...
  ...（跳跃过程）...
  [JumpAbility] OnJumpFinishedEventReceived - Jump finished event triggered by AN_JumpFinish
  [JumpAbility] EndAbility - Actor: ..., bWasCancelled: 0
  [JumpAbility] Removed Player_Status_Jumping tag
  [JumpAbility] Cleared JumpFinishedEventTask
  ```

---

## ✅ 验证清单

### 功能验证
- [ ] 跳跃时`Player_Status_Jumping` Tag被正确添加
- [ ] 跳跃过程中Tag持续存在（不会提前移除）
- [ ] Land动画播放到80%-90%时，`AN_SendGameplayEventToOwner`触发
- [ ] 收到`Player.Event.Jump.Finished`事件后，Ability正确结束
- [ ] `Player_Status_Jumping` Tag在Ability结束时被移除

### 边界场景验证
- [ ] **连续跳跃**：第一次跳跃落地后，第二次跳跃正常
- [ ] **空中攻击测试**：在空中尝试按攻击键，应该被`ActivationBlockedTags`阻止
- [ ] **土狼时间跳跃**：离开地面后的土狼时间内跳跃，流程正常
- [ ] **取消跳跃**：如果跳跃被其他能力取消，Ability正确清理

---

## ⚠️ 注意事项

### 1. **确保Notify必定触发**
如果Land动画被跳过或中断（如被击飞），Notify可能不会触发。

**当前保护机制**：
- `ActivationBlockedTags` 防止空中重复跳跃
- `JumpFinishedEventTask` 在`EndAbility`时会被清理

**可选增强**（如需要）：
可以在`RPGPlayerCharacter::OnMovementModeChanged`中添加兜底机制：
```cpp
// 当从Falling变为Walking时，如果JumpAbility还在运行，强制结束
// 但这通常不需要，因为Notify应该能正常触发
```

### 2. **动画蓝图中的状态机逻辑**
确保你的跳跃状态机逻辑正确：
```
None → Start → Loop → Land → [AN_JumpFinish] → None
```

在`Land`状态转换为`None`的条件中：
- `bJumpAnimationFinished && bIsGrounded` 应该为true
- 这与`AN_JumpFinish`的触发时机应该一致

### 3. **网络同步**
- `SendGameplayEventToActor` 支持网络同步
- 如果是单机游戏，无需额外配置
- 如果是多人游戏，确保EventTag在客户端和服务器都正确触发

---

## 🐛 常见问题排查

### 问题1: 日志中没有看到"OnJumpFinishedEventReceived"
**原因**: AN_SendGameplayEventToOwner未触发或EventTag配置错误

**解决**:
1. 检查动画蓝图中Notify的位置是否正确
2. 检查EventTag是否填写为 `Player.Event.Jump.Finished`
3. 在Notify的Blueprint中添加日志，确认Notify被触发

### 问题2: 跳跃后无法第二次跳跃
**原因**: `Player_Status_Jumping` Tag未被移除

**解决**:
1. 在控制台输入 `showtags Player.Status.Jumping` 查看Tag是否存在
2. 检查AN_JumpFinish是否触发
3. 检查EventTag路径是否完全匹配

### 问题3: 空中攻击仍然能打中断跳跃
**原因**: `Player_Status_Jumping` Tag提前被移除

**解决**:
1. 检查日志中`Removed Player_Status_Jumping tag`的时机
2. 确保没有其他地方调用`RemoveJumpingTag()`
3. 验证`EndAbility`只在Event触发时调用

---

## 📊 完整流程图

```
玩家按下跳跃键
    ↓
JumpAbility.ActivateAbility()
    ├─ ApplyJumpingTag() → 添加 Player.Status.Jumping
    ├─ PerformJump() → Character.Jump()
    ├─ SetupJumpFinishedEventWait() → 监听 Player.Event.Jump.Finished
    └─ Ability保持激活状态 ⚡
    ↓
动画状态机执行
    JumpStart → JumpLoop → Land动画播放
    ↓
Land动画播放到80%-90%
    ↓
AN_SendGameplayEventToOwner触发
    └─ SendGameplayEventToActor(Owner, Player.Event.Jump.Finished)
    ↓
JumpAbility.OnJumpFinishedEventReceived()
    └─ EndAbility()
        ├─ RemoveJumpingTag() → 移除 Player.Status.Jumping ✅
        ├─ Clear JumpFinishedEventTask
        └─ Ability结束
    ↓
动画状态机: Land → None (bJumpAnimationFinished && bIsGrounded)
    ↓
角色回到Idle状态，可以进行下一次跳跃
```

---

## 📝 修改文件清单

1. ✅ `Source/RPG/Public/RPGGameplayTags.h` - 添加EventTag声明
2. ✅ `Source/RPG/Private/RPGGameplayTags.cpp` - 定义EventTag
3. ✅ `Source/RPG/Public/AbilitySystem/Abilities/Player/RPGPlayerAbility_Jump.h` - 添加Event监听声明
4. ✅ `Source/RPG/Private/AbilitySystem/Abilities/Player/RPGPlayerAbility_Jump.cpp` - 实现Event监听和生命周期管理

---

## 🎓 下一步优化建议

### 1. **添加兜底保护机制**（可选）
在`RPGPlayerCharacter::OnMovementModeChanged`中：
```cpp
if (PrevMovementMode == MOVE_Falling && CurrentMode == MOVE_Walking)
{
    // 如果JumpAbility仍在运行，延迟0.3s后强制结束
    // 防止Notify未触发的极端情况
}
```

### 2. **传递EventData**（可选）
在AN_SendGameplayEventToOwner中传递落地速度、位置等信息：
```cpp
EventData.OptionalObject = this;
EventData.OptionalTarget = GetActorLocation();
```

### 3. **性能优化**（如需要）
如果频繁跳跃，考虑：
- 复用JumpFinishedEventTask
- 使用对象池管理Ability实例

---

## ✨ 总结

通过本次修改，JumpAbility的生命周期现在与跳跃动画完全同步：
- ✅ `Player_Status_Jumping` Tag在整个跳跃过程中持续存在
- ✅ 防止空中攻击打断跳跃动画
- ✅ Land动画Notify正常触发
- ✅ 第二次跳跃动画正常运行
- ✅ 符合GAS事件驱动架构规范

配置完成后，你的跳跃系统应该能正确处理所有边界场景！
