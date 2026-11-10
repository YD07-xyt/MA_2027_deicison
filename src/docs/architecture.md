# RM Decision 决策模块架构文档

> RoboMaster 2027 MA 哨兵决策模块

## 1 模块概述

`rm_decision` 是一个基于 **BehaviorTree.CPP** 的哨兵机器人决策模块。它接收游戏数据和导航数据作为输入，通过行为树进行决策推理，输出导航目标点和哨兵姿态模式。

### 1.1 数据流

```
serial/socket → game_data ─┐
                            ├→ bt 行为树决策 → ros2 导航
nav → nav_data ────────────┘
```

**输入**：`Game2Decision` + `Nav2Decision`
**输出**：`Decision2Game` + `Decision2Nav`

## 2 目录结构

```
rm_decision/
├── include/rm_decision/          # 公开接口（Public API）
│   ├── api.hpp                   # 数据结构定义（输入/输出、枚举）
│   ├── config.hpp                # 配置结构体 & Point 类型
│   ├── rm_decision.hpp           # 总入口 RmDecision 类
│   ├── log.hpp                   # 日志工具
│   ├── custume_types.hpp         # 自定义类型（已弃用）
│   ├── action/
│   │   ├── intput_blackboard.hpp # 外部数据写入黑板的适配层
│   │   └── output_blackboard.hpp # 黑板数据读取输出的适配层
│   └── ros2/
│       └── ros2_node.hpp         # ROS2 节点封装（已弃用）
├── src/                          # 私有实现（Private）
│   ├── rm_decision.cpp           # RmDecision 实现
│   ├── action/                   # Action 节点
│   │   ├── nav2pose.hpp          # 导航目标点发送
│   │   ├── async_sleep.hpp       # 异步等待
│   │   ├── switch_nav_model.hpp  # 导航模式切换
│   │   └── chase.hpp             # 追击逻辑
│   └── condition/                # Condition 节点
│       ├── is_game_start.hpp     # 比赛开始检测
│       ├── is_game_time_exceeded.hpp # 超时检测
│       ├── is_gmae_time_less.hpp # 时间不足检测
│       ├── is_go_home.hpp        # 回家条件
│       ├── is_robot_health.hpp   # 机器人健康检测
│       ├── is_enemy_outpost_destroyed.hpp # 敌方哨站摧毁检测
│       ├── is_fort_occupied.hpp  # 基地被占领检测
│       ├── is_chase.hpp          # 追击条件
│       ├── check_blackboard_bool.hpp # 黑板布尔值检测
│       ├── check_value.hpp       # 数值比较检测
│       └── set_blackboard_bool.hpp # 黑板布尔值设置
├── behavior_tree/                # 行为树 XML 定义
│   ├── rmuc2027.xml              # 2027 赛季主树
│   ├── rmuc2026.xml              # 2026 赛季主树
│   ├── subtree/                  # 子树定义
│   │   ├── hit_enemy_outpost_subtree.xml # 攻击敌方哨站
│   │   └── patrol_subtree.xml    # 巡逻子树
│   └── test/                     # 测试用树
├── map/
│   └── RM2025.png                # 场地地图
├── CMakeLists.txt
├── README.md
└── TODO.md
```

## 3 核心类

### 3.1 RmDecision（总入口）

文件：[`rm_decision.hpp`](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/include/rm_decision/rm_decision.hpp)

| 方法 | 说明 |
|------|------|
| `tree_tick(game_data, nav_data, timeout)` | 将数据注入黑板上一次行为树执行 |
| `get_data()` | 从黑板上读取决策结果 |
| `setting_tree_custom_node()` | 向工厂注册所有自定义节点 |

核心流程：
1. 构造函数加载 XML 行为树文件，注册自定义节点和枚举
2. 每次 `tree_tick()` 将外部数据写入黑板，执行一次 tick
3. `get_data()` 从黑板取出决策结果（导航目标 + 姿态模式）

### 3.2 数据结构

文件：[`api.hpp`](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/include/rm_decision/api.hpp)

| 结构体 | 说明 |
|--------|------|
| `Game2Decision` | 游戏数据输入：HP、弹药、敌方哨站血量、游戏时间、当前位置、追击坐标、基地占领状态等 |
| `Nav2Decision` | 导航状态输入 |
| `Decision2Game` | 决策输出到游戏：哨兵模式（攻击/防御/移动） |
| `Decision2Nav` | 决策输出到导航：目标点坐标 |
| `SentryModel` | 哨兵模式枚举：`ATTACK(1)` / `DEFENSE(2)` / `MOVE(3)` |
| `NavState` | 导航状态枚举：`IDLE` / `FAILURE` / `SUCCEEDED` / `RUNNING` |

### 3.3 黑板适配层

- **Intputblackboard**：[`intput_blackboard.hpp`](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/include/rm_decision/action/intput_blackboard.hpp) — 将 `Game2Decision` 和 `Nav2Decision` 写入行为树黑板的各个键值
- **Outputblackboard**：[`output_blackboard.hpp`](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/include/rm_decision/action/output_blackboard.hpp) — 从黑板上读取 `goal` 和 `sentry_model` 并返回决策结果

### 3.4 Config

文件：[`config.hpp`](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/include/rm_decision/config.hpp)

| 字段 | 说明 |
|------|------|
| `tree_xml_file` | 行为树 XML 文件路径 |
| `tree_node_model_export_path` | 节点模型导出路径 |
| `pub_goal_topic_name` | 导航目标话题名 |
| `nav2_state_topic_name` | 导航状态话题名 |
| `map_tf_name` | 地图 TF 坐标系名 |
| `send_goal_timeout` | 发送目标超时时间（ms） |

## 4 行为树节点一览

### 4.1 Action 节点

| 节点名 | 文件 | 类型 | 说明 |
|--------|------|------|------|
| `Nav2Pose` | [nav2pose.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/action/nav2pose.hpp) | StatefulAction | 导航目标点发送，支持 seen_active 防残留状态 |
| `AsyncSleep` | [async_sleep.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/action/async_sleep.hpp) | StatefulAction | 异步等待（不阻塞 BT） |
| `SwitchNavModel` | [switch_nav_model.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/action/switch_nav_model.hpp) | SyncAction | 切换导航模式（follow/spinning/downhill） |

### 4.2 Condition 节点

| 节点名 | 文件 | 说明 |
|--------|------|------|
| `IsGameStart` | [is_game_start.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_game_start.hpp) | 比赛是否开始 |
| `IsGameTimeExceeded` | [is_game_time_exceeded.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_game_time_exceeded.hpp) | 当前时间 > 阈值 |
| `IsGameTimeLess` | [is_gmae_time_less.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_gmae_time_less.hpp) | 当前时间 < 阈值 |
| `IsGoHome` | [is_go_home.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_go_home.hpp) | 是否需要回家（低血量/低弹药） |
| `IsRobotHealth` | [is_robot_health.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_robot_health.hpp) | 机器人是否健康（高血量/充足弹药） |
| `IsEnemyOutpostDestroyed` | [is_enemy_outpost_destroyed.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_enemy_outpost_destroyed.hpp) | 敌方哨站是否已摧毁 |
| `IsFortOccupied` | [is_fort_occupied.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_fort_occupied.hpp) | 我方基地是否被占领 |
| `IsChase` | [is_chase.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/is_chase.hpp) | 是否需要追击 |
| `CheckValue` | [check_value.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/check_value.hpp) | 通用数值比较 |
| `CheckBlackboardBool` | [check_blackboard_bool.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/check_blackboard_bool.hpp) | 黑板布尔检查 |
| `SetBlackboardBool` | [set_blackboard_bool.hpp](file:///home/gan/NAV_WS/ma_decision/MA_2027_deicison/src/rm_decision/src/condition/set_blackboard_bool.hpp) | 黑板布尔设置 |

## 5 决策逻辑分层

决策采用三层优先级架构，按优先级从高到低：

### 5.1 安全层（最高优先级）

**触发条件**：`HP ≤ 60` **且** `弹药 ≤ 10`

**行为**：导航回原点 `(0, 0, 1.0 rad)`，等待 4s 后重新评估

### 5.2 战术选择层

**触发条件**：`弹药 ≥ 70` **且** `HP ≥ 350`（健康状态）

按 `ReactiveFallback` 优先级递减执行：

1. **攻击敌方哨站** — 条件：敌方哨站血量 ≤ 40（未摧毁）
   - 行为：导航至 `(2.65, 4.36, 1.0)`，每 2s 重新评估
2. **保护英雄** — 条件：游戏时间在 400~420s
   - 行为：导航至 `(2.65, 4.36, 1.0)`，每 2s 重新评估
3. **巡逻**（保底任务） — 无条件
   - 行为：无限循环 4 点巡逻，每个点超时 24~25s

### 5.3 巡逻点位

| 序号 | 坐标 | 超时 |
|------|------|------|
| 点1 | `(2.18, 3.20, 1.0)` | 24s |
| 点2 | `(2.15, -0.73, 1.0)` | 25s |
| 点3 | `(8.56, 3.07, 1.0)` | 25s |
| 点4 | `(10.6, -0.54, 1.0)` | 25s |

## 6 BT 执行状态含义

| 颜色 | 状态 | 说明 |
|------|------|------|
| 🟦 蓝色 | SUCCESS | 节点执行成功 |
| 🟥 红色 | FAILURE | 节点执行失败 |
| 🟩 绿色 | RUNNING | 节点正在执行中（如导航中） |
| ⬜ 灰色 | IDLE | 节点尚未被执行 |
| 🟨 黄色 | HALTED | 节点被强制终止 |

## 7 依赖

- **BehaviorTree.CPP**：行为树框架（v4 格式）
- **spdlog**：日志库
- **Eigen3**：矩阵/线性代数
- **ROS2 (Humble/Iron)**：`rclcpp`, `geometry_msgs`, `action_msgs`, `nav2_msgs`, `tf2_eigen`, `nav2_common`
- **tools**：内部工具库（`tools/logger.hpp`）

## 8 开发规范

1. **公开头文件**放在 `include/rm_decision/`，**私有头文件**放在 `src/` 下
2. `rm_decision.hpp/.cpp` 是唯一的总入口，外部只通过它交互
3. 输入/输出数据结构统一在 `api.hpp` 定义，保证模块出入口单一
4. 新增节点先在 `src/` 下按分类编写，再到 `setting_tree_custom_node()` 中注册
5. 优先编写原子逻辑节点，复杂逻辑由 BT 子树组装
6. 外部数据变更时，同步更新 `intput_blackboard.hpp` 和 `output_blackboard.hpp`