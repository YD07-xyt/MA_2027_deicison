# 决策行为树（Behavior Tree）XML 说明

本文档总结 `behavior_tree/` 目录下所有行为树 XML 的用途、决策逻辑及其对应的 RoboMaster 比赛。

## 1. 总览

### 1.1 目录结构

```
behavior_tree/
├── rmuc2026.xml                        # RMUC 2026 正式版主树
├── rmuc2027.xml                        # 超级对抗赛决策（RMUC 2027）
├── rmul.2027.xml                       # 联盟赛决策（RMUL 2027，空壳）
├── subtree/
│   ├── hit_enemy_outpost_subtree.xml   # 攻击敌方哨站子树（独立拆分版）
│   └── patrol_subtree.xml              # 巡逻子树（独立拆分版）
└── test/
    ├── rmuc2026.xml                    # 测试版 2026（红方视角）
    ├── main_tree.xml                   # 测试版 2027「高地赛制」完整逻辑
    ├── patrol.xml                      # 最简巡逻测试
    └── generated_models.xml            # Groot 节点模型描述（非行为树）
```

### 1.2 命名规则与比赛对应

| 前缀/后缀 | 含义 |
|-----------|------|
| `rmuc` | RMUC（RoboMaster University Championship，高校系列赛） |
| `rmul` | RMUL（RoboMaster University League，高校联盟赛） |
| `2026` / `2027` | 赛季年份 |
| 根目录 `behavior_tree/` | 正式版本 |
| `test/` | 开发 / 测试版本 |

机器人定位：**哨兵机器人（Sentry）**，自主导航，负责巡逻、防守基地、攻击敌方前哨站、占领高地。

### 1.3 文件清单

| 文件 | 赛季 | 视角 | 状态 |
|------|------|------|------|
| `rmuc2026.xml` | RMUC 2026 | 蓝方 | 正式版 |
| `rmuc2027.xml` | 超级对抗赛（RMUC 2027） | 蓝方 | 正式版 |
| `rmul.2027.xml` | 联盟赛（RMUL 2027） | — | 空壳，未实现 |
| `subtree/hit_enemy_outpost_subtree.xml` | — | 蓝方 | 子树拆分，暂未被主树引用 |
| `subtree/patrol_subtree.xml` | — | 蓝方 | 子树拆分，暂未被主树引用 |
| `test/rmuc2026.xml` | RMUC 2026 | 红方 | 测试版（旧接口写法） |
| `test/main_tree.xml` | RMUC 2027 高地赛制 | 红方 | 测试版（含上下坡） |
| `test/patrol.xml` | — | — | 最小导航验证 |
| `test/generated_models.xml` | — | — | Groot 节点端口定义 |

---

## 2. 主树通用决策框架

正式版主树（`rmuc2026.xml` / `rmuc2027.xml`）采用**三层优先级决策**：

```
Sequence
├── IsGameStart                 # 比赛开始
└── ReactiveFallback            # 优先级回退，按顺序尝试
    ├── [1] 回家序列（最高优先级）
    │      条件：弹药低 且 血量低
    │      动作：导航回基地 → 等待
    ├── [2] 健康判断 + 战术层
    │      条件：弹药充足 且 血量充足
    │      └── ReactiveFallback
    │          ├── hit_enemy_outpost_subtree   # 攻击敌方哨站
    │          ├── protect_hero_subtree        # 保护英雄
    │          └── patrol_subtree              # 兜底巡逻
```

| 层级 | 触发条件 | 行为 |
|------|---------|------|
| 安全层（回家） | 弹药 ≤ 10 且 血量 ≤ 60 | 回基地 `(0,0,1.0)`，等 4s |
| 健康判断 | 弹药 ≥ 70 且 血量 ≥ 350 | 进入战术层 |
| 战术层① 攻击哨站 | 敌方哨站血量 ≤ 40 | 去 `(2.65,4.36)`，等 2s |
| 战术层② 保护英雄 | 游戏时间窗口 | 去 `(2.65,4.36)`，等 2s |
| 战术层③ 巡逻（兜底） | 无条件 | 4 点循环巡逻 |

---

## 3. 各文件详细说明

### 3.1 rmuc2026.xml —— RMUC 2026 正式版（蓝方）

节点风格：使用专用条件节点 `IsGoHome` / `IsRobotHealth` / `IsEnemyOutpostDestroyed`。

- **回家序列**：`IsGoHome`（弹药≤10 且血量≤60）后追加两个 `CheckValue` 双保险 → `Nav2Pose(0,0,1.0)` → `Sleep 4000`。
- **健康序列**：`IsRobotHealth`（弹药≥70 且血量≥350）→ 战术层。
- **hit_enemy_outpost_subtree**：`Inverter(IsEnemyOutpostDestroyed(min=40))` 表示“哨站未被摧毁（血量>40）”时执行 → 导航 `(2.65,4.36)` → 睡 2s。
- **protect_hero_subtree**：`IsGameTimeExceeded(420)` + `IsGameTimeLess(420)` → 导航 `(2.65,4.36)`（⚠️ 两个阈值均为 420，实际永不满足，见「已知问题」）。
- **patrol_subtree**：4 点循环巡逻，`Repeat num_cycles="-1"`，每点 `Timeout` + `Sleep 1000`。

巡逻点：`(2.18,3.2)`、`(2.15,-0.73)`、`(8.56,3.07)`、`(10.6,-0.54)`。

### 3.2 rmuc2027.xml —— 超级对抗赛决策（RMUC 2027）

结构同 2026，但**全部改用通用节点 `CheckValue`**（`le` ≤ / `ge` ≥）替代专用条件节点：

- 回家：`CheckValue(弹药≤10)` + `CheckValue(血量≤60)`。
- 健康：`CheckValue(弹药≥70)` + `CheckValue(血量≥350)`。
- 攻击哨站：`CheckValue(哨站血量≤40)`。
- 保护英雄：`CheckValue(game_time≤420)` + `CheckValue(game_time≥400)`，即 400~420s 窗口。
- 巡逻点与 2026 一致。

### 3.3 rmul.2027.xml —— 联盟赛决策（RMUL 2027）

仅一行注释 `<!-- 联盟赛决策 -->`，**尚未实现**，为联盟赛预留的占位文件。

### 3.4 subtree/ 独立子树

- `hit_enemy_outpost_subtree.xml`：与 `rmuc2027.xml` 内联的同名子树一致（`CheckValue(哨站血量≤40)` → 导航 → 睡 2s）。
- `patrol_subtree.xml`：4 点巡逻，坐标与正式版一致。

两者是为“子树拆分复用”准备的独立文件，但当前主树 XML 仍以内联方式定义同名子树，**未真正 include 这两个文件**。

### 3.5 test/rmuc2026.xml —— 测试版 2026（红方视角）

逻辑与正式版 2026 相同，差异：

- 坐标全部为**红方**视角：回家 `(-0.476,-6.84)`、攻哨站 `(8.79,2.24)`、巡逻 `(5.92,0.108)` 等。
- 使用旧接口写法 `pub_model="publisher"`、`use_blackboard`。
- `protect_hero_subtree` 时间窗口为 `IsGameTimeExceeded(1)` + `IsGameTimeLess(420)`（几乎全程）。

### 3.6 test/main_tree.xml —— 测试版 2027「高地赛制」完整逻辑（红方）

体现 2027 赛季「高地/资源岛」玩法，引入上下坡与导航模式切换（`SwitchNavModel`）和黑板标志（`SetBlackboardBool` / `CheckBlackboardBool`）。

- **go_home（回家）**：若未下坡 `downhill_done=false` → 切 `downhill` 模式从坡顶 `(5.5,4.43)` 下到坡下 `(6.17,2.79)` → 切 `spinning` → 最后回基地 `(1.3,-6.9,0.0)`。
- **uphill_then_protect（上坡后保护高地）**：
  - 分支A（上坡，仅一次）：时间 395~420s → 坡下 `(6.17,2.79)` → `SwitchNavModel(follow)` 跟随云台上坡 → 坡顶 `(5.5,4.43)` → `SwitchNavModel(spinning)` 小陀螺 → 置 `uphill_done=true`。
  - 分支B（保护，持续）：`uphill_done=true` 且时间 350~420s → 持续导航坡顶 `(5.5,4.43)` + 短暂等待保持驻守。
- **down_hill_subtree（下坡）**：切 `downhill` 从 `(5.5,4.43)` 下到 `(6.17,2.79)` → 切 `spinning`。
- **hit_enemy_outpost_subtree**：攻哨站（时间条件被写成 420~420，实际失效）。
- **patrol_subtree**：巡逻 + 复位 `downhill_done/uphill_done` 标志。

`SwitchNavModel` 模式含义：`follow`（跟随云台）、`spinning`（小陀螺自转）、`downhill`（下坡）。

### 3.7 test/patrol.xml —— 最简巡逻测试

`SequenceWithMemory` + 两个 `Timeout` + `Nav2Pose`，仅用于最小化验证导航节点。

### 3.8 test/generated_models.xml —— Groot 节点模型（非行为树）

`TreeNodesModel` 声明各节点（`Nav2Pose`、`IsGoHome`、`IsRobotHealth`、`SwitchNavModel`、`CheckBlackboardBool` 等）的端口定义，供 **Groot 可视化编辑器**识别，不参与实际决策。

---

## 4. 已知问题

1. **`CheckValue` 节点未在代码中注册**：`rmuc2026.xml`、`rmuc2027.xml`、`subtree/hit_enemy_outpost_subtree.xml` 均使用 `CheckValue`，但 `rm_decision.cpp` 未注册该节点，加载这些树会抛异常。
2. **`rmuc2026.xml` 保护英雄时间窗口 bug**：`IsGameTimeExceeded(420)` + `IsGameTimeLess(420)` 同时为 420，永不满足；`rmuc2027.xml` 已修正为 400~420。
3. **`test/main_tree.xml` 攻击哨站时间条件失效**：`IsGameTimeExceeded(420)` + `IsGameTimeLess(420)` 同为 420。
4. **`rmul.2027.xml` 为空壳**：联盟赛决策尚未实现。
5. **`subtree/` 独立子树未被主树引用**：主树仍内联定义，存在两份重复逻辑。
6. **红蓝方坐标不一致**：正式版为蓝方坐标（回家 `(0,0)`），测试版为红方坐标（回家 `(-0.476,-6.84)`），实际部署需按己方颜色切换。