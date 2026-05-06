# 2 相关理论及技术基础

## 2.1 虚幻引擎骨骼动画系统理论

骨骼动画是第三人称角色扮演游戏实现角色动作表现的核心技术，其目标是通过骨骼层级变换与动画状态管理，实现角色行为的自然流畅表现。UE5采用蒙皮骨骼系统（Skinned Mesh）与动画状态机（Animation State Machine）相结合的方案，为高质量角色动画提供底层支撑。若采用传统逐帧动画方案，虽然能够实现基本的动画播放，但也存在诸多问题。

一方面，传统逐帧动画需要为每个角色动作独立制作完整的动画序列，这会导致动画资源数量呈指数级增长，尤其是在本课题所开发的第三人称RPG游戏中，角色需要支持移动、攻击、跳跃、受击等多种行为状态，资源管理成本显著增大。另一方面，逐帧动画缺乏状态间的平滑过渡机制，当角色从一种行为状态切换到另一种状态时（如从Idle状态切换到Run状态），若直接切换动画序列，会导致视觉上的突变和跳跃感，严重影响玩家的游戏体验。此外，逐帧动画方案难以实现不同角色间的动画复用，当项目中存在多个角色模型时，需要为每个角色单独制作动画，开发效率低下。

针对本课题第三人称RPG游戏对角色动画的高质量需求，采用UE5骨骼动画系统结合动画状态机方案，通过Animation Blueprint实现动画逻辑与渲染逻辑的解耦，利用2D BlendSpace实现多方向移动动画的平滑混合，确保角色行为与动画表现的精准匹配。

## 2.2 骨骼动画算法原理

骨骼动画算法的核心目标在于通过骨骼层级变换计算网格顶点位置，实现角色姿态的实时更新。其核心算法基于正向运动学（Forward Kinematics, FK）与逆向运动学（Inverse Kinematics, IK）的混合计算。

算法核心为：骨骼变换矩阵的层级传递方程：

M_WORLD(n) = M_WORLD(PARENT(n)) × M_LOCAL(n)  （公式2.1）

其中：
- M_WORLD(n)：第n个骨骼的全局变换矩阵
- M_WORLD(PARENT(n))：父骨骼的全局变换矩阵
- M_LOCAL(n)：第n个骨骼的局部变换矩阵（包含平移、旋转、缩放）

以M_WORLD代表骨骼的全局变换状态，M_LOCAL代表骨骼相对于父骨骼的局部变换。保证角色网格顶点V在动画帧中的正确位置，需要满足如下条件：

（1）正向运动学（FK）计算，从根骨骼开始逐层向下计算子骨骼的全局变换。FK的计算过程是自上而下的递归过程，每个骨骼的位置和姿态由其父骨骼决定。以角色手臂为例，肩关节的旋转会带动肘关节和腕关节的整体移动，肘关节的旋转会带动腕关节的移动。这种层级关系确保了骨骼运动的自然性和连贯性。在实际实现中，UE5通过AnimInstance类在每个Tick中执行FK计算，将动画序列中的骨骼变换数据应用到骨骼层级结构上。

（2）逆向运动学（IK）计算，根据目标位置反向求解骨骼姿态。与FK不同，IK的计算方向是从末端效应器（End Effector）向根部反向求解。以角色脚部落地为例，当角色站在不平整的地面上时，需要根据地面高度反向计算腿部骨骼的弯曲角度，确保脚部与地面完全接触。IK算法通过雅可比矩阵（Jacobian Matrix）迭代求解，公式为：

Δθ = J⁺(θ) × Δx  （公式2.2）

其中J⁺为雅可比矩阵的伪逆，Δx为末端效应器的目标位移，Δθ为骨骼关节角度的调整量。UE5提供了CCDIK（Cyclic Coordinate Descent IK）和TwoBoneIK两种求解器，分别适用于多关节链和双关节链的IK计算。

（3）IK/FK混合权重控制，通过权重系数w实现FK与IK的平滑过渡：

M_FINAL = w × M_IK + (1 - w) × M_FK  （公式2.3）

权重w的取值范围为[0,1]，当w=0时完全使用FK结果，当w=1时完全使用IK结果。通过动画状态机中的Transition Graph控制权重变化，实现不同动画状态间的平滑过渡。例如，当角色从行走状态切换到瞄准状态时，IK权重从0逐渐增加到1，使角色上半身从FK动画过渡到IK瞄准姿态。

### 2.2.1 动画状态机理论

动画状态机（Animation State Machine）是管理角色动画状态转换的核心机制，其通过有限状态机（Finite State Machine, FSM）模型实现动画状态的有序管理。状态机包含以下核心元素：

（1）状态（State）：每个状态对应一个动画序列或动画混合逻辑。本课题中的角色动画状态包括Idle（待机）、Walk（行走）、Run（奔跑）、Jump（跳跃）、Attack（攻击）等。每个状态内部可配置动画播放速率、循环模式、根运动（Root Motion）同步等参数。

（2）转换（Transition）：状态之间的切换规则由Transition Graph定义，转换条件通常基于角色属性（如移动速度、是否在空中）或外部事件（如攻击指令）。UE5通过Rule-Based Transition机制，在每帧评估转换条件，当条件满足时触发状态切换，并根据配置的Transition Duration执行动画混合。

（3）过渡混合（Transition Blending）：当从一个状态切换到另一个状态时，UE5使用线性插值（Lerp）或样条插值（Spline）在过渡时间内混合两个动画。混合公式为：

POSE_OUTPUT = Lerp(POSE_OLD, POSE_NEW, α(t/T))  （公式2.4）

其中POSE_OLD为旧状态动画输出，POSE_NEW为新状态动画输出，α为归一化时间参数（0到1），T为过渡时间。通过混合机制消除状态切换时的视觉突变，确保动画流畅性。

### 2.2.2 动画重定向技术

动画重定向（Animation Retargeting）是将动画从一个骨架映射到另一个不同骨架的技术，其核心在于骨骼映射（Skeleton Mapping）与比例适配。UE5通过IK Retargeter实现动画重定向，主要解决以下问题：

（1）骨骼拓扑差异，不同角色模型可能具有不同的骨骼数量和层级结构。重定向技术通过定义源骨架与目标骨架的骨骼对应关系，建立映射表（Mapping Table），确保动画数据能够正确传递。例如，源角色的"Spine01"骨骼可能对应目标角色的"Spine"骨骼。

（2）骨骼比例差异，不同角色模型的身高、臂展等比例可能存在显著差异。直接应用动画会导致目标角色出现姿态异常（如脚部悬空、手臂穿透身体）。重定向技术通过比例缩放因子（Scale Factor）调整骨骼变换，确保动画在不同比例角色上的正确表现。比例缩放基于骨骼长度比值计算：

Scale = Length_TARGET_BONE / Length_SOURCE_BONE  （公式2.5）

通过为每个骨骼链计算独立的缩放因子，重定向技术能够适配不同体型的角色模型，实现动画资源的最大化复用。本课题中的角色系统通过统一的骨架标准，确保所有角色模型能够共享同一套动画资源，显著降低开发成本。

## 2.3 UE5导航系统与AI寻路算法

导航系统是实现AI角色智能移动的核心组件，其目标是为AI提供从起点到目标点的最优路径计算能力。UE5采用导航网格（Navigation Mesh, NavMesh）结合A*搜索算法的方案，为大规模场景中的实时寻路提供高效支撑。若采用网格导航（Grid-Based Navigation）方案，虽然实现简单，但也存在诸多局限性。

一方面，网格导航将场景划分为均匀的方格，每个方格作为寻路的基本单元。在复杂地形场景中，网格导航需要极高的分辨率才能精确表示可行走区域，导致导航数据量呈平方级增长，内存消耗显著增大。另一方面，网格导航生成的路径通常为折线，需要通过额外的后处理（如路径平滑）才能得到自然的移动轨迹，增加了计算开销。此外，网格导航难以准确表示斜坡、楼梯等非平面地形，AI在这些区域的移动表现往往不自然。

针对本课题RPG游戏中AI角色需要在复杂地形中执行巡逻、追逐、逃跑等行为的需求，采用NavMesh结合A*算法的方案，NavMesh通过多边形网格精确表示可行走区域，A*算法提供最优路径搜索，确保AI寻路的准确性与高效性。

## 2.4 NavMesh A*寻路算法原理

A*寻路算法的核心目标是在图搜索过程中找到从起点到目标点的最优路径。其算法结合了Dijkstra算法的全局最优性和贪心最佳优先搜索（Greedy Best-First Search）的启发式引导，通过代价函数评估节点优先级。

算法核心为：节点代价评估函数：

f(n) = g(n) + h(n)  （公式2.6）

其中：
- f(n)：节点n的总代价估计值
- g(n)：从起点到节点n的实际代价
- h(n)：从节点n到目标点的启发式估计代价（启发函数）

A*算法的执行过程基于优先队列（Open List）管理待探索节点，具体步骤如下：

（1）初始化阶段，将起点加入Open List，设置起点的g值为0，f值为启发函数计算结果。同时维护Closed List记录已探索节点，避免重复处理。在NavMesh导航中，起点和目标点首先需要投影到NavMesh多边形上，确定其所在的导航节点。

（2）节点扩展阶段，从Open List中选择f值最小的节点n作为当前节点。若n为目标节点，则搜索结束，通过回溯父节点指针构建完整路径。否则，将n从Open List移至Closed List，并遍历n的所有相邻节点m。对于每个相邻节点m，计算从起点经过n到达m的新代价：

g_new(m) = g(n) + cost(n, m)  （公式2.7）

其中cost(n, m)为从节点n到节点m的移动代价，在NavMesh中通常基于多边形中心距离或边权重计算。若m不在Open List中，或g_new(m)小于m当前的g值，则更新m的父节点为n，更新g(m)和f(m)，并将m加入Open List。

（3）启发函数设计，h(n)的设计直接影响A*算法的性能和路径质量。常用的启发函数包括：

- 欧几里得距离：h(n) = √[(x_n - x_goal)² + (y_n - y_goal)²]  （公式2.8）
- 曼哈顿距离：h(n) = |x_n - x_goal| + |y_n - y_goal|  （公式2.9）

欧几里得距离适用于允许任意方向移动的场景，而曼哈顿距离适用于仅允许上下左右移动的场景。在NavMesh导航中，由于AI可以在多边形内自由移动，通常采用欧几里得距离作为启发函数。为保证A*算法找到最优路径，启发函数必须满足可接纳性（Admissibility）条件，即h(n)不能高估实际代价。

### 2.4.1 动态避障算法

在实时游戏场景中，AI角色需要在移动过程中避开动态障碍物（如其他角色、移动物体）。UE5结合Reciprocal Velocity Obstacles（RVO）算法实现局部避障。

RVO算法的核心思想是：每个智能体独立计算避障速度，避免碰撞的同时保持向目标移动。算法基于速度空间（Velocity Space）分析，计算安全速度范围：

VO_A|B = {v_A | ∃t > 0, ||(p_A + v_A × t) - (p_B + v_B × t)|| ≤ r_A + r_B}  （公式2.10）

其中：
- v_A、v_B：智能体A和B的速度向量
- p_A、p_B：智能体A和B的位置向量
- r_A、r_B：智能体A和B的半径

VO_A|B表示会导致A与B碰撞的速度集合。RVO算法通过选择不在VO_A|B中的速度，或选择碰撞时间最长的速度，实现避障。多个智能体同时应用RVO算法时，通过迭代协商达到全局无碰撞状态。该算法的计算复杂度为O(n²)，适用于小规模智能体群体的实时避障。

## 2.5 AI行为决策系统

AI行为决策系统是实现AI角色多样化行为表现的核心框架，其目标是根据游戏状态和感知信息，选择并执行合适的行为。UE5采用行为树（Behavior Tree, BT）结合黑板（Blackboard）的方案，为AI行为逻辑提供可视化编辑和灵活扩展能力。若采用有限状态机（FSM）实现AI行为，虽然概念简单，但也存在诸多问题。

一方面，FSM的状态数量随行为复杂度呈指数级增长。以本课题中的敌人为例，若需要实现巡逻、警戒、追逐、攻击、逃跑、死亡等多种行为，FSM需要定义大量状态和状态转换规则，导致状态图极其复杂，难以维护和调试。另一方面，FSM难以实现行为的优先级管理和并发执行。例如，当AI在追逐玩家过程中检测到自身血量过低时，需要中断追逐行为并执行逃跑行为，FSM需要为每种行为切换添加显式的转换逻辑，代码冗余度高。

针对本课题RPG游戏中AI角色需要实现巡逻、追逐、攻击、逃跑等多样化行为的需求，采用行为树方案，通过树形结构组织行为逻辑，支持优先级管理和动态行为切换，提升AI行为的可配置性和可扩展性。

## 2.5.1 行为树算法原理

行为树是一种层次化的决策树结构，通过节点组合实现复杂行为的模块化定义。行为树包含以下核心节点类型：

（1）控制流节点（Control Flow Nodes），用于控制子节点的执行顺序和条件。常见的控制流节点包括：

- Sequence节点：按顺序执行所有子节点，若某个子节点返回Failure，则整个Sequence返回Failure；若所有子节点返回Success，则Sequence返回Success。
- Selector节点：按顺序尝试执行子节点，若某个子节点返回Success，则整个Selector返回Success；若所有子节点都返回Failure，则Selector返回Failure。
- Parallel节点：并发执行所有子节点，根据配置的Policy决定返回条件（如任一子节点成功则返回成功，或所有子节点成功才返回成功）。

（2）执行节点（Execution Nodes），用于执行具体的行为逻辑。常见的执行节点包括：

- Action节点：执行具体的操作（如移动、攻击、播放动画），返回Success、Failure或Running（表示操作正在进行中）。
- Condition节点：检查某个条件是否满足（如玩家是否在感知范围内、血量是否低于阈值），返回Success或Failure。
- Decorator节点：装饰器节点用于修改子节点的行为，如Inverter节点反转子节点的返回结果，Repeater节点重复执行子节点指定次数。

行为树的执行过程采用Tick机制，游戏引擎每帧调用根节点的Tick方法，递归遍历子节点。黑板（Blackboard）作为行为树的全局数据存储，用于在节点间共享信息（如目标玩家位置、警戒状态）。黑板采用键值对（Key-Value）存储结构，支持多种数据类型（Vector、Float、Bool、Object等）。节点通过读取和写入黑板数据，实现状态传递和决策依据获取。

### 2.5.2 AI感知系统理论

AI感知系统（AI Perception System）是AI获取环境信息的核心组件，其通过模拟人类感官（视觉、听觉等）为行为树决策提供数据支撑。UE5的AIPerception系统基于感知配置（Sight、Hearing、Damage等）实现多维度感知。

感知系统的核心机制包括：

（1）视觉感知（Sight Perception），基于锥形检测区域（Sight Cone）检测目标。检测条件包括：

- 距离检测：目标与AI的距离必须在感知半径（Sight Radius）内。
- 角度检测：目标必须位于AI的视野角度（Peripheral Vision Angle）范围内。
- 视线检测（Line of Sight）：AI与目标之间不能有遮挡物（通过射线检测验证）。
- 团队检测：通过TeamId系统区分敌友关系。本课题中，为检测中立团队，需启用bDetectNeutrals配置。

（2）听觉感知（Hearing Perception），基于噪声源检测机制。当场景中发生噪声事件（如枪声、脚步声）时，通过UAISense_Hearing注册噪声源，AI根据噪声位置和响度（Loudness）调整感知状态。听觉感知不受视线遮挡影响，使AI能够感知视线外的威胁。

（3）感知更新机制，感知系统按配置的更新频率（Auto Success Report Time）扫描环境，将检测结果存储到黑板中供行为树使用。感知结果包括目标位置、最后已知位置、感知状态（Seen、Heard、Damaged）等信息，行为树节点通过读取这些数据执行相应的行为逻辑。

## 2.6 CommonUI MVVM架构原理

UI系统是游戏与玩家交互的核心接口，其设计质量直接影响玩家的游戏体验。UE5的CommonUI框架采用MVVM（Model-View-ViewModel）架构模式，通过分层设计实现UI逻辑与业务逻辑的解耦。若采用传统的紧耦合UI设计，虽然实现快速，但也存在诸多问题。

一方面，紧耦合UI将UI显示逻辑与数据处理逻辑混合在同一类中，当数据发生变化时，需要手动更新UI元素，导致代码冗余且容易遗漏更新。以本课题中的HUD为例，若生命值、法力值、武器图标等多个UI元素都需要根据游戏状态实时更新，紧耦合方案需要在每个数据变化点调用UI更新方法，代码维护成本显著增大。另一方面，紧耦合UI难以实现UI组件的复用和独立测试，当需要在不同场景中复用同一UI组件时，往往需要复制大量代码，违背了软件工程中的DRY（Don't Repeat Yourself）原则。

针对本课题RPG游戏中对UI系统的高内聚低耦合需求，采用CommonUI框架结合四层栈架构，通过委托机制（Delegate）和组件化设计实现数据与视图的分离，提升UI系统的可维护性和可扩展性。

## 2.6.1 四层栈UI架构理论

CommonUI框架采用四层栈架构管理不同类型的UI界面，通过层级优先级确保UI显示的正确性。四层架构包括：

（1）Game层（Game Layer），用于显示主游戏界面，如HUD、得分显示、小地图等。Game层位于最底层，始终显示在游戏场景中，为玩家提供核心游戏信息。本课题中的URPGHUDWidget即部署在Game层，负责实时显示玩家生命值、法力值等关键信息。

（2）LocalPlayer层（LocalPlayer Layer），用于显示玩家专属的交互界面，如背包、技能栏、装备界面等。LocalPlayer层位于Game层之上，仅在玩家触发相关操作时显示。该层支持多玩家分屏场景，每个玩家拥有独立的LocalPlayer层。

（3）Modal层（Modal Layer），用于显示模态对话框，如设置菜单、暂停界面、确认对话框等。Modal层位于LocalPlayer层之上，显示时会屏蔽下层UI的交互，确保玩家优先处理模态对话框的内容。Modal层通过栈结构管理多个对话框，支持对话框的层级堆叠。

（4）Overlay层（Overlay Layer），用于显示覆盖层，如加载提示、错误提示、成就通知等。Overlay层位于最顶层，通常具有半透明背景或动画效果，确保玩家注意到提示信息。Overlay层的UI元素通常具有较短的显示时间，显示后自动消失。

UI层的管理通过UI Manager子系统（如URPGUIManagerSubsystem）实现，通过PushWidgetToLayerStack方法将Widget推送到指定层。HUD显示必须通过该方法推送到Game层，确保UI层级关系的正确性。

### 2.6.2 数据绑定与观察者模式

MVVM架构的核心在于数据绑定（Data Binding）机制，其通过观察者模式（Observer Pattern）实现Model与View的自动同步。观察者模式定义了一对多的依赖关系，当Model发生变化时，所有依赖它的View都会自动收到通知并更新。

在本课题的健康系统与UI桥接层架构中，观察者模式的应用如下：

（1）Model层（URPGHealthComponent），负责管理生命值数据，通过ASC（Ability System Component）监听属性变化。当生命值发生变化时，HealthComponent通过动态多播委托（Dynamic Multicast Delegate）OnHealthChanged广播事件。委托声明如下：

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, NewHealth, float, OldHealth);

该委托支持多个监听器订阅，所有订阅者都会在委托触发时收到回调。

（2）ViewModel层（URPGPlayerUIComponent），作为数据桥接组件，订阅HealthComponent的委托事件，并将数据转换为UI可用的格式。PlayerUIComponent在BeginPlay阶段初始化订阅关系，通过AddDynamic方法绑定委托回调：

HealthComponent->OnHealthChanged.AddDynamic(this, &URPGPlayerUIComponent::OnHealthChanged);

当收到HealthChanged事件时，PlayerUIComponent触发自己的OnManaChangedForUI委托，将事件传递给View层。

（3）View层（URPGHUDWidget），负责UI元素的显示和更新。HUDWidget在Initialize阶段获取PlayerUIComponent引用，并订阅其委托事件。当收到数据变化通知时，HUDWidget更新对应的UI元素（如HealthBar进度条、HealthText文本）。

通过这种三层分离架构，健康数据的管理、转换和显示分别由不同组件负责，实现了高内聚低耦合的设计目标。当需要修改UI显示逻辑时，只需修改HUDWidget，不影响HealthComponent的数据管理逻辑。

## 2.7 动态多播委托在UI层的应用原理

动态多播委托（Dynamic Multicast Delegate）是UE中实现事件驱动编程的核心机制，其支持多个函数订阅同一事件，并在事件触发时依次调用所有订阅函数。与静态委托相比，动态委托支持蓝图绑定，是实现C++与蓝图通信的关键技术。

### 2.7.1 委托生命周期管理

动态多播委托的生命周期管理是确保内存安全和避免内存泄漏的关键。UE提供了多种委托绑定方式，每种方式适用于不同场景：

（1）AddDynamic绑定，通过UObject指针绑定成员函数，委托调用时会检查对象是否有效。绑定语法为：

Delegate.AddDynamic(Object, &UMyClass::MyFunction);

AddDynamic要求回调函数必须标记为UFUNCTION()，否则会导致编译错误。该绑定方式适用于UObject派生类的成员函数绑定，委托调用时会自动检查对象有效性。

（2）AddWeakLambda绑定，通过TWeakObjectPtr捕获对象指针的弱引用，适用于需要捕获UObject的Lambda表达式。然而，动态多播委托不支持AddWeakLambda，这是由于动态多播委托的序列化机制和蓝图兼容性要求，无法处理Lambda的闭包捕获。若需要使用Lambda绑定，应使用静态多播委托（DECLARE_MULTICAST_DELEGATE）。

（3）委托清理，当Widget停用时（NativeOnDeactivated），必须清理委托绑定，避免悬空指针和无效回调。清理方法为：

Delegate.RemoveAll(this);

或在订阅时保存DelegateHandle，通过Remove方法移除特定绑定：

Delegate.Remove(DelegateHandle);

### 2.7.2 UI更新性能优化

在UI系统中，委托回调的性能优化直接影响游戏的帧率和响应性。优化策略包括：

（1）批量更新，避免在委托回调中逐帧更新UI元素。例如，当生命值频繁变化时（如每秒减少10次），不应每次都更新HealthBar，而应采用节流（Throttle）机制，限制更新频率（如每秒最多更新5次）。

（2）线程安全，所有UI更新必须在游戏线程（Game Thread）执行。若委托在异步线程（如异步加载回调）中触发，需通过AsyncTask(ENamedThreads::GameThread, ...)将UI更新调度到游戏线程，避免线程竞争和渲染异常。

（3）按需更新，仅更新发生变化的UI元素。例如，当生命值变化时，仅更新HealthBar和HealthText，不更新ManaBar和WeaponIcon。通过委托参数传递变化数据（如NewHealth、OldHealth），View层根据数据判断需要更新的元素。

## 2.8 系统开发工具

Unreal Engine 5.6：Epic Games开发的第三代游戏引擎，采用Nanite虚拟化微多边形几何体系统和Lumen全动态全局光照系统，为高质量第三人称RPG游戏提供图形渲染底层支撑。引擎内置蓝图可视化编程系统、C++编译工具链、动画状态机编辑器、行为树编辑器等开发工具，支持从原型设计到产品发布的完整开发流程。本课题基于UE5.6版本开发，利用其Gameplay Ability System（GAS）实现角色能力系统，通过CommonUI框架构建UI架构，采用NavMesh导航系统实现AI寻路。

Rider for Unreal Engine：JetBrains开发的跨平台IDE，提供智能代码补全、实时错误检测、重构工具、调试器等高级功能。Rider深度集成UE的UHT（Unreal Header Tool）和UBT（Unreal Build Tool），支持C++与蓝图的混合调试，显著提升开发效率。本课题采用Rider作为主要C++开发工具，利用其代码导航功能快速定位类和函数定义，通过断点调试追踪委托回调和数据流。

Git版本控制工具：分布式版本控制系统，用于管理项目源代码和配置文件的版本历史。通过分支管理（Branch Management）实现功能开发与主线开发的隔离，通过提交（Commit）记录每次代码变更，通过合并（Merge）整合不同开发者的工作成果。本课题采用Git进行版本控制，结合.gitignore配置排除Intermediate、Saved等自动生成目录，确保仓库体积可控。

Visual Studio Code：轻量级源代码编辑器，通过C/C++插件提供语法高亮、智能提示、代码格式化等功能。VSCode通过compileCommands.json配置IntelliSense路径，支持UE项目的代码补全和错误检测。本课题采用VSCode作为辅助编辑工具，用于快速查看和编辑配置文件、Markdown文档等。

Blender：开源三维建模软件，支持角色建模、骨骼绑定、动画制作、UV展开等功能。通过FBX导出格式将模型和动画导入UE5，用于角色和场景资源的制作。本课题采用Blender处理3D模型资源，确保模型拓扑结构符合UE5的骨骼动画要求。