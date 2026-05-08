# 整体模块化架构图

```mermaid
graph TB
    subgraph 输入与控制模块
        Input[增强输入系统]
        PCtrl[玩家控制器]
        AICtrl[AI控制器]
    end
    
    subgraph 动画模块
        Anim[动画系统]
        StateMachine[状态机]
    end
    
    subgraph 健康与UI模块
        Health[健康组件]
        UIBridge[UI桥接层]
        HUD[HUD显示]
    end
    
    subgraph AI模块
        Perception[感知系统]
        BT[行为树]
    end
    
    subgraph 战斗与武器模块
        Combat[战斗组件]
        Weapon[武器系统]
        Combo[连招管理]
    end
    
    subgraph 能力系统模块
        ASC[能力系统组件]
        AttrSet[属性集]
        GA[GameplayAbility]
        Damage[伤害计算]
    end
    
    subgraph 对象池模块
        Pool[敌人对象池]
        Lifecycle[生命周期管理]
    end
    
    %% 输入与控制流
    Input --> PCtrl
    PCtrl --> Anim
    PCtrl --> Combat
    Input --> ASC
    AICtrl --> BT
    Perception --> AICtrl
    BT --> Combat
    
    %% 动画流
    Anim --> StateMachine
    Combat --> Anim
    
    %% 健康与UI流
    Health --> UIBridge
    UIBridge --> HUD
    Health --> ASC
    
    %% 战斗流
    Combat --> Weapon
    Combat --> Combo
    Combat --> ASC
    Weapon --> ASC
    
    %% 能力系统流
    ASC --> AttrSet
    ASC --> GA
    ASC --> Damage
    Damage --> AttrSet
    
    %% 对象池流
    Pool --> Lifecycle
    Lifecycle --> Combat
    Lifecycle --> AICtrl
    
    %% 跨模块交互
    Health -.委托.-> UIBridge
    UIBridge -.UI事件.-> Input
```

**图 整体模块化架构图**

## 模块交互说明

### 1. 输入与控制流
- **玩家输入**：增强输入系统 → 玩家控制器 → 动画系统/战斗组件
- **能力输入**：增强输入系统 → 能力系统组件（通过GameplayTag）
- **AI控制**：感知系统 → AI控制器 → 行为树 → 战斗组件

### 2. 动画驱动流
- **移动动画**：玩家控制器 → 动画系统 → 状态机
- **战斗动画**：战斗组件 → 动画系统 → Linked Anim Layers

### 3. 健康与UI流
- **数据监听**：健康组件 → UI桥接层（委托） → HUD显示
- **属性同步**：健康组件 ↔ 能力系统组件（属性集）
- **输入屏蔽**：UI事件 → 输入系统（通过CommonUI输入路由）

### 4. 战斗与能力流
- **攻击触发**：战斗组件 → 武器系统 → 能力系统
- **连招管理**：战斗组件 → 连招管理 → 能力系统
- **伤害计算**：能力系统 → 伤害计算 → 属性集

### 5. 对象池管理流
- **敌人激活**：对象池 → 生命周期管理 → 战斗组件/AI控制器
- **敌人回收**：健康组件（死亡） → 对象池 → 生命周期管理

## 通信机制

### 动态多播委托
- 健康状态变化：`OnHealthChanged`
- UI数据更新：`OnHealthChangedForUI`
- 武器命中事件：`OnHitTargetActor`

### GameplayTag系统
- 输入标签：`Input.Move`、`Input.LightAttack`、`Input.HeavyAttack`
- 能力标签：`Ability.EquipSword`、`Ability.Fireball`
- 状态标签：`State.Attacking`、`State.Dead`

### 接口交互
- `IPawnUIInterface`：UI数据访问接口
- `IPawnCombatInterface`：战斗组件访问接口
- `IPawnDeathInterface`：死亡事件接口
