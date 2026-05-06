# 4 详细设计（续）

## 4.4 控制器模块详细设计

### 4.4.1 控制器架构设计

控制模块负责协调输入模块、动画模块、健康系统模块和UI系统模块的工作，实现角色的整体控制逻辑。控制模块采用控制器（Controller）架构，通过ARPGPlayerController实现玩家控制，通过ARPGEnemyAIController实现AI控制。

控制器的核心职责包括接收输入信号、协调组件工作流、管理角色状态。ARPGPlayerController处理玩家特有的控制逻辑，如HUD显示控制、主菜单调用、玩家团队ID管理。ARPGBaseController封装控制器的通用逻辑，供玩家控制器和AI控制器继承，避免代码重复。

在玩家控制器中，BeginPlay方法是初始化的关键入口。该方法首先调用父类的BeginPlay完成基础初始化，然后通过GetGameInstance()->GetSubsystem<URPGUIManagerSubsystem>()获取UI管理器实例，调用ShowHUD方法创建并显示HUD界面。这种设计确保了HUD在游戏开始时自动显示，无需手动管理。

团队ID管理是控制器的重要功能之一。通过GetGenericTeamId方法返回玩家的团队ID，用于AI感知系统的敌友识别。玩家团队ID设置为0（中立团队），AI感知系统通过启用bDetectNeutrals配置检测中立团队，确保敌人能够感知到玩家。

输入屏蔽控制是控制器的另一关键功能。当UI界面打开时，控制器暂时屏蔽角色的游戏输入，避免玩家同时操作角色和UI。CommonUI框架通过BlockInput机制自动处理输入屏蔽，控制器无需手动实现。当UI界面关闭时，自动恢复角色控制输入。

### 4.4.2 控制器类图

控制器详细类图如图4.10所示。

## 4.5 动画模块详细设计

### 4.5.1 动画系统架构设计

动画模块负责管理角色的动画状态机、动画混合空间和动画蒙太奇，实现角色行为与动画表现的精准匹配。动画模块通过Animation Blueprint实现动画逻辑，通过AnimInstance类提供动画数据接口。

动画系统的核心组件包括URPGCharacterAnimInstance（角色动画实例）和URPGBaseAnimInstance（基础动画实例）。URPGBaseAnimInstance封装通用的动画逻辑，如获取角色引用、计算移动速度和方向、更新空中状态等。URPGCharacterAnimInstance继承自URPGBaseAnimInstance，添加角色特有的动画逻辑，如攻击蒙太奇播放、移动方向计算等。

动画状态机（Animation State Machine）是管理角色动画状态转换的核心机制。状态机包含Idle、Walk、Run、Jump、Attack等状态，状态之间的转换由转换规则（Transition Rule）控制。转换规则基于角色属性（如移动速度、是否在空中）判断是否满足转换条件。当移动速度从0增加到100时，状态机从Idle状态切换到Walk状态；当移动速度超过300时，从Walk状态切换到Run状态。

2D BlendSpace实现移动方向的动画混合。BlendSpace的X轴表示移动速度，Y轴表示移动方向（前进、后退、左移、右移）。根据角色的实际移动速度和方向，BlendSpace插值计算最终的动画姿态。这种混合机制确保角色在各个方向上的移动动画都能平滑过渡，避免动画突变。

动画蒙太奇（Animation Montage）用于播放非循环的动画片段，如攻击动画、受击动画等。通过PlaySlotAnimationAsDynamicMontage方法播放攻击动画，支持动画分段（Section）和通知（Notify）。动画通知用于触发攻击判定、音效播放等事件，实现动画与游戏逻辑的同步。

根运动（Root Motion）同步机制确保动画播放与角色物理移动的一致性。当动画包含根运动时，角色的位置由动画驱动，而非物理系统。根运动同步避免滑步现象，提升动画表现的真实感。在URPGBaseAnimInstance的NativeUpdateAnimation方法中，每帧更新动画参数，根据角色的最新状态调整动画播放。

### 4.5.2 动画模块类图

动画模块类图如图4.12所示。

## 4.6 AI模块详细设计

### 4.6.1 AI系统架构设计

AI模块负责实现敌人的智能行为，采用行为树（Behavior Tree）和黑板（Blackboard）架构，通过AIPerception系统感知玩家位置和环境信息。AI模块通过ARPGEnemyAIController控制AI行为逻辑，实现巡逻、追逐、攻击、逃跑等多样化行为。

AI系统的核心组件包括ARPGEnemyAIController（AI控制器）、UBehaviorTree（行为树）、UBlackboardComponent（黑板组件）和UAIPerceptionComponent（感知组件）。ARPGEnemyAIController负责协调各组件的工作流，行为树定义AI的行为逻辑，黑板存储AI的状态数据，感知组件检测玩家和环境信息。

行为树是一种层次化的决策树结构，通过节点组合实现复杂行为的模块化定义。行为树包含控制流节点（Control Flow Nodes）和执行节点（Execution Nodes）。控制流节点包括Sequence节点（按顺序执行所有子节点）、Selector节点（按顺序尝试执行子节点）和Parallel节点（并发执行所有子节点）。执行节点包括Action节点（执行具体的操作）、Condition节点（检查某个条件是否满足）和Decorator节点（修改子节点的行为）。

黑板作为行为树的全局数据存储，用于在节点间共享信息。黑板采用键值对（Key-Value）存储结构，支持多种数据类型（Actor、Vector、Float、Bool等）。节点通过读取和写入黑板数据，实现状态传递和决策依据获取。例如，感知系统将检测到的玩家位置写入黑板的TargetLocation键，行为树的MoveTo节点读取该键值执行寻路。

AI感知系统（AI Perception System）是AI获取环境信息的核心组件。感知系统基于感知配置（Sight、Hearing、Damage等）实现多维度感知。视觉感知（Sight Perception）基于锥形检测区域检测目标，检测条件包括距离检测、角度检测、视线检测和团队检测。听觉感知（Hearing Perception）基于噪声源检测机制，当场景中发生噪声事件时，AI根据噪声位置和响度调整感知状态。

感知系统按配置的更新频率扫描环境，将检测结果存储到黑板中供行为树使用。感知结果包括目标位置、最后已知位置、感知状态（Seen、Heard、Damaged）等信息，行为树节点通过读取这些数据执行相应的行为逻辑。当玩家进入感知范围时，触发OnTargetPerceptionUpdated回调，将玩家信息写入黑板，行为树执行追逐和攻击行为。

### 4.6.2 AI模块类图

AI模块类图如图4.9所示。

## 4.7 输入模块详细设计

### 4.7.1 输入系统架构设计

输入模块基于UE5增强输入系统（Enhanced Input）实现，相比旧版输入系统（Legacy Input），增强输入系统提供了更强大的输入映射、输入修饰符（Input Modifier）和输入触发器（Input Trigger）机制。

输入模块的核心组件包括Input Mapping Context（IMC）、Input Action（IA）、Input Modifier和Input Trigger。Input Mapping Context定义输入动作与输入源的映射关系，将键盘WASD键映射到Move动作，将空格键映射到Jump动作，将鼠标左键映射到Attack动作。IMC支持多输入源绑定，如同时支持键盘和手柄输入。

Input Action定义输入动作的类型和参数。Move动作采用Axis2D类型，返回二维向量（X轴控制前后移动，Y轴控制左右移动）。Jump动作采用Boolean类型，返回按下/释放状态。Attack动作采用Boolean类型，支持连击检测。

Input Modifier对输入信号进行预处理。Dead Zone修饰器用于处理摇杆死区，避免摇杆轻微偏移导致角色移动。Scale修饰器用于缩放输入值，调整移动速度或灵敏度。Negate修饰器用于反转输入轴，适配不同的控制习惯。

Input Trigger定义输入动作的触发条件。Pressed触发器在按键按下时触发一次。Released触发器在按键释放时触发一次。Hold触发器在按键持续按下时持续触发。Tap触发器检测快速点击，用于区分单击和长按。

UEnhancedInputComponent是增强输入组件，负责绑定输入动作到回调函数。在ARPGPlayerCharacter的SetupPlayerInputComponent方法中，通过BindAction方法将输入动作绑定到对应的处理函数。例如，BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARPGPlayerCharacter::OnMoveInput)将Move动作绑定到OnMoveInput方法。

输入模块通过输入优先级标签（Input Priority Tag）实现输入屏蔽。当UI界面打开时，UI层通过BlockInput机制屏蔽角色控制输入，避免玩家同时操作角色和UI。当UI界面关闭时，恢复角色控制输入。输入优先级的管理通过CommonUI框架自动处理，无需手动实现。

## 4.8 系统交互流程详细设计

### 4.8.1 玩家受击完整流程

玩家受击完整流程如图4.19所示。

当敌人执行攻击动画并触发攻击判定通知时，调用ApplyDamage方法对玩家造成伤害。玩家的ASC执行GameplayEffect，修改Health属性。HealthComponent监听到属性变化，更新CurrentHealth并触发OnHealthChanged委托。

UIComponent收到健康变化通知，触发OnHealthChangedForUI委托。HUD Widget收到通知后，调用UpdateHealth方法更新HealthBar进度条和HealthText文本。如果生命值降至0，HealthComponent调用StartDeath方法，播放死亡动画并触发OnDeathStarted委托。UIComponent收到死亡通知后，显示死亡界面。

### 4.8.2 UI界面切换流程

UI界面切换流程如图4.20所示。

当玩家按下Escape键时，PlayerController调用UIManager的ShowMainMenu方法。UIManager创建URPGMainMenuWidget实例，推送到Menu层。主菜单激活后，通过BlockInput机制屏蔽Game层的输入，HUD仍然显示但无法交互。

当玩家点击"Resume"按钮时，主菜单调用UIManager的HideMainMenu方法。UIManager停用主菜单Widget，恢复Game层的输入响应。主菜单从Menu层弹出，HUD重新获得输入焦点，玩家可以继续游戏。

## 4.9 配置与资源管理详细设计

### 4.9.1 输入映射配置

输入映射配置定义了输入动作与输入源的映射关系。Move动作映射到WASD键和左摇杆，采用Axis2D类型，配置Dead Zone和Scale修饰器，使用Triggered触发器。Jump动作映射到空格键和手柄底部按钮，采用Boolean类型，使用Pressed触发器。Attack动作映射到鼠标左键和右扳机键，采用Boolean类型，使用Pressed触发器。Menu动作映射到Escape键和Start按钮，采用Boolean类型，使用Pressed触发器。

### 4.9.2 健康属性配置

健康属性配置定义了角色的健康相关属性。Health属性类型为Float，初始值为100.0，最小值为0.0，最大值为999.0，表示当前生命值。MaxHealth属性类型为Float，初始值为100.0，最小值为1.0，最大值为999.0，表示最大生命值。Mana属性类型为Float，初始值为50.0，最小值为0.0，最大值为999.0，表示当前法力值。MaxMana属性类型为Float，初始值为50.0，最小值为1.0，最大值为999.0，表示最大法力值。

### 4.9.3 UI层栈配置

UI层栈配置定义了四层栈的结构和行为。Game层显示URPGHUDWidget，不屏蔽输入，在游戏开始时显示，游戏结束时移除。Menu层显示URPGMainMenuWidget，屏蔽输入，在按Escape键时显示，点击Resume/Exit时移除。Modal层显示设置/确认对话框，屏蔽输入，在点击设置按钮时显示，点击关闭按钮时移除。Overlay层显示加载提示/通知，不屏蔽输入，在触发事件时显示，事件结束时移除。
