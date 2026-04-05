# RPG游戏 - ASC混合架构设计文档

## 概述

本项目采用混合架构设计来管理Ability System Component (ASC)：

- **玩家角色**：使用 `PlayerState + ASC` 架构
- **敌人角色**：使用 `Character + ASC` 架构

这种设计结合了两种方案的优势，既保证了玩家数据的持久性和网络稳定性，又简化了AI敌人的实现复杂度。

## 架构设计

### 1. 玩家架构 (PlayerState + ASC)

#### 核心类
- **ARPGPlayerState**: 玩家的GameState，包含ASC和AttributeSet
- **ARPGPlayerCharacter**: 玩家的角色表现层，负责动画、输入等

#### 优势
1. **数据持久性**: PlayerState在关卡切换时保持不变，适合保存玩家等级、技能点等持久化数据
2. **网络稳定性**: PlayerState的复制更加稳定，减少网络抖动对玩家数据的影响
3. **符合Epic最佳实践**: 官方推荐的做法，便于后续扩展和维护
4. **分离关注点**: 将游戏逻辑（PlayerState）与表现层（Character）分离

#### 实现细节

```cpp
// ARPGPlayerState 包含 ASC
URPGAbilitySystemComponent* RPGAbilitySystemComponent;
URPGAttributeSet* RPGAttributeSet;

// 初始化时将Avatar设置为Player Character
RPGAbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
```

#### 数据流
```
Player Controller → Player State (ASC) → Player Character (Avatar)
                        ↓
                  属性同步到客户端
```

### 2. 敌人架构 (Character + ASC)

#### 核心类
- **AEnemyCharacter**: 敌人基类，直接在Character上包含ASC

#### 优势
1. **简化实现**: 不需要额外的PlayerState，减少类的数量
2. **降低复杂度**: 敌人不需要持久化数据，生命周期跟随Character
3. **性能优化**: 减少不必要的网络复制开销
4. **易于管理**: 敌人生成和销毁时，ASC自动创建和清理

#### 实现细节

```cpp
// AEnemyCharacter 直接在Character上包含ASC
URPGAbilitySystemComponent* RPGAbilitySystemComponent;
URPGAttributeSet* RPGAttributeSet;

// 初始化时将Avatar设置为自己
RPGAbilitySystemComponent->InitAbilityActorInfo(this, this);
```

#### 数据流
```
AI Controller → Enemy Character (ASC + Avatar)
                     ↓
               属性同步到客户端
```

## 统一访问接口

### URPGFunctionLibrary

提供了统一的ASC访问函数，自动处理玩家和敌人的差异：

```cpp
URPGAbilitySystemComponent* URPGFunctionLibrary::NativeGetRPGASCFromActor(AActor* InActor)
{
    // 首先尝试从Actor直接获取ASC（敌人）
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);
    
    // 如果未找到，尝试从PlayerState获取（玩家）
    if (!ASC)
    {
        if (const APawn* Pawn = Cast<APawn>(InActor))
        {
            if (const APlayerState* PlayerState = Pawn->GetPlayerState())
            {
                ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
            }
        }
    }

    return CastChecked<URPGAbilitySystemComponent>(ASC);
}
```

### 动画实例

动画实例也支持从两个位置获取ASC：

```cpp
void URPGBaseAnimInstance::NativeInitializeAnimation()
{
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwningCharacter)
    {
        // 首先尝试从Character获取ASC（敌人）
        AbilitySystemComponent = OwningCharacter->FindComponentByClass<UAbilitySystemComponent>();
        
        // 如果未找到，尝试从PlayerState获取（玩家）
        if (!AbilitySystemComponent)
        {
            if (APlayerState* PlayerState = OwningCharacter->GetPlayerState())
            {
                IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState);
                if (ASI)
                {
                    AbilitySystemComponent = ASI->GetAbilitySystemComponent();
                }
            }
        }
    }
}
```

## GameMode配置

在 `ARPGGameModeBase` 中设置默认的PlayerState类：

```cpp
ARPGGameModeBase::ARPGGameModeBase()
{
    PlayerStateClass = ARPGPlayerState::StaticClass();
}
```

## 网络复制配置

### 玩家ASC复制
- ASC设置在PlayerState上，自动参与网络复制
- 属性变化会自动同步到所有客户端
- PlayerState的复制优先级更高，更稳定

### 敌人ASC复制
- ASC设置在EnemyCharacter上
- 随Character一起复制
- 敌人销毁时自动清理

## 使用指南

### 创建新玩家角色

1. 继承 `ARPGPlayerCharacter`
2. 通过 `GetPlayerState<ARPGPlayerState>()` 获取ASC
3. 不要直接在Character上添加ASC

```cpp
ARPGPlayerState* PlayerState = GetPlayerState<ARPGPlayerState>();
URPGAbilitySystemComponent* ASC = PlayerState->GetRPGAbilitySystemComponent();
```

### 创建新敌人

1. 继承 `AEnemyCharacter`
2. 直接使用 `GetRPGAbilitySystemComponent()` 获取ASC

```cpp
URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponent();
```

### 在蓝图中访问ASC

使用 `URPGFunctionLibrary` 提供的函数：

```blueprint
Get RPG ASC From Actor (Target Actor) → Add Gameplay Tag
```

这个函数会自动处理玩家和敌人的差异。

## 注意事项

1. **不要在Player Character上直接添加ASC**：这会导致重复的ASC实例
2. **使用统一接口访问ASC**：始终使用 `URPGFunctionLibrary::NativeGetRPGASCFromActor()`
3. **网络复制测试**：确保在多人游戏中正确测试属性同步
4. **保存/加载系统**：玩家数据应从PlayerState的AttributeSet读取

## 未来扩展

1. **添加更多AttributeSet**：可以为不同角色类型创建专门的AttributeSet
2. **实现存档系统**：基于PlayerState的AttributeSet实现玩家数据持久化
3. **优化网络带宽**：根据重要性调整ASC的复制频率
4. **添加AI专用功能**：在AEnemyCharacter中添加AI特定的能力系统功能

## 参考资源

- [Unreal Engine GAS Documentation](https://docs.unrealengine.com/en-US/GameplayAbilitySystem/index.html)
- [Epic Games Best Practices](https://www.unrealengine.com/en-US/blog/best-practices-for-gameplay-ability-system)
