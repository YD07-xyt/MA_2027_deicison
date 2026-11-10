# 联盟赛（RMUL）决策实现指南

> 目标：在现有代码基础上，**复用已注册节点**，仅新增行为树 XML，实现联盟赛决策，不改动 C++ 节点代码。

## 1. 核心思路

BehaviorTree.CPP 将「节点」和「决策逻辑」分离：

- **节点（C++）** = 积木，已注册进 `factory_`，可无限复用；
- **行为树（XML）** = 图纸，描述决策逻辑。

因此，新增联盟赛决策 = **新建一棵行为树 XML**，用现成节点拼装，无需写任何新 C++。

## 2. 可复用节点盘点

### 2.1 内置控制节点（无需注册，直接写 XML 即可用）

`Sequence`（顺序）、`ReactiveFallback`（优先级回退）、`Inverter`（取反）、`Repeat`（循环）、`Timeout`（超时）、`Sleep`（延时）、`SubTree`（子树调用）、`SequenceWithMemory`（记忆式顺序）。

### 2.2 已注册自定义节点（可直接用）

| 类型 | 节点 | 端口 | 用途 |
|------|------|------|------|
| Action | `Nav2Pose` | `goal`/`nav_state`/`goal2nav` | 发布导航目标点 |
| Action | `SwitchNavModel` | `model` | 切换姿态（跟随/小陀螺/下坡） |
| Action | `AsyncSleep` | — | 异步延时 |
| Condition | `IsGameStart` | `current_game_state` | 比赛是否开始 |
| Condition | `IsGoHome` | `current_projectile_allowance`/`min_projectile_allowance`/`current_hp`/`min_hp` | 是否需要回家 |
| Condition | `IsRobotHealth` | `current_projectile_allowance`/`min_projectile_allowance`/`current_hp`/`min_hp` | 是否健康 |
| Condition | `IsEnemyOutpostDestroyed` | `enemy_outpost_hp`/`min_enemy_outpost_hp` | 敌方前哨站状态 |
| Condition | `IsFortOccupied` | `ours_fort_occ_state` | 我方基地是否被占领 |
| Condition | `IsGameTimeExceeded` | `game_time`/`threshold` | 时间大于阈值 |
| Condition | `IsGameTimeLess` | `game_time`/`threshold` | 时间小于阈值 |
| Condition | `CheckBlackboardBool` | — | 读黑板布尔值 |
| Condition | `SetBlackboardBool` | — | 写黑板布尔值 |

### 2.3 未注册节点（不可用，必须避开）

- `CheckValue`（通用数值比较）—— 未注册，XML 中使用会导致加载失败；
- `Chase` / `IsChase`（追击）—— 未注册。

## 3. 黑板可用键（XML 里 `{xxx}` 引用）

由 `Intputblackboard::set_data()` 写入，行为树可直接引用：

| 键名 | 类型 | 含义 | 是否有数据来源 |
|------|------|------|----------------|
| `current_hp` | int | 当前血量 | ✅ 串口 |
| `is_game_start` | bool | 比赛开始 | ✅ 串口 |
| `ours_fort_occ_state` | bool | 我方基地占领 | ⚠️ 仅 socket 模式映射 |
| `game_time` | int | 比赛时间 | ✅ 串口 |
| `current_enemy_outpost_hp` | int | 敌方前哨站血量 | ✅ 串口映射 |
| `current_projectile_allowance` | int | 可发弹量 | ✅ 串口 |
| `nav_state` | NavState | 导航状态 | ✅ nav_feedback |
| `current_pose` | Point | 当前位置 | ❌ 无来源（恒 0,0,0） |
| `chase_pose` | Point | 追击位置 | ❌ 无来源（恒 0,0,0） |

## 4. 需要创建/修改的文件

| 文件 | 位置 | 说明 |
|------|------|------|
| 联盟赛决策树 | `src/rm_decision/behavior_tree/rmul_2027.xml` | 核心（已存在空壳，直接填内容） |
| 子树（可选） | `src/rm_decision/behavior_tree/subtree/rmul_xxx_subtree.xml` | 如需拆分子树 |
| 配置修改 | `config/param.yaml` | `tree_xml_file` + `is_decision` |
| 说明文档 | `src/rm_decision/docs/rmul_decision.md` | 本文件 |

## 5. 决策树骨架示例

复用 `rmuc2026.xml` 的三层框架，但**去掉未注册的 `CheckValue`**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<root BTCPP_format="4" main_tree_to_execute="main_tree">
  <BehaviorTree ID="main_tree">
    <Sequence>
      <IsGameStart current_game_state="{is_game_start}"/>
      <ReactiveFallback>
        <!-- 1. 回家（最高优先级） -->
        <Sequence>
          <IsGoHome current_projectile_allowance="{current_projectile_allowance}"
                    min_projectile_allowance="10"
                    current_hp="{current_hp}"
                    min_hp="60"/>
          <Nav2Pose goal="0.0,0.0,1.0" nav_state="{nav_state}" goal2nav="{goal}"/>
          <Sleep msec="4000"/>
        </Sequence>
        <!-- 2. 健康后进入战术层 -->
        <Sequence>
          <IsRobotHealth current_projectile_allowance="{current_projectile_allowance}"
                         min_projectile_allowance="70"
                         current_hp="{current_hp}"
                         min_hp="350"/>
          <ReactiveFallback>
            <SubTree ID="hit_enemy_outpost_subtree"
                     current_enemy_outpost_hp="{current_enemy_outpost_hp}"
                     game_time="{game_time}"
                     _autoremap="true"/>
            <SubTree ID="protect_hero_subtree"
                     game_time="{game_time}"
                     _autoremap="true"/>
            <SubTree ID="patrol_subtree" _autoremap="true"/>
          </ReactiveFallback>
        </Sequence>
      </ReactiveFallback>
    </Sequence>
  </BehaviorTree>

  <!-- 子树定义：攻前哨 / 护英雄 / 巡逻（阈值、坐标需按联盟赛规则替换） -->
  <BehaviorTree ID="hit_enemy_outpost_subtree"> ... </BehaviorTree>
  <BehaviorTree ID="protect_hero_subtree"> ... </BehaviorTree>
  <BehaviorTree ID="patrol_subtree"> ... </BehaviorTree>
</root>
```

## 6. 配置切换

修改 [config/param.yaml](../../../config/param.yaml)：

```yaml
decision:
  is_decision: true   # 打开决策
  tree_xml_file: "src/rm_decision/behavior_tree/rmul_2027.xml"  # 指向联盟赛树
```

## 7. 注意事项

1. **禁止使用未注册节点**（`CheckValue`/`Chase`/`IsChase`），否则 `createTreeFromFile` 抛异常。
2. **`is_decision` 必须改 `true`**，否则 `game_data` 不填充，树 tick 到全是默认值。
3. **位置数据无来源**：`current_pose`/`chase_pose` 恒为 `(0,0,0)`，联盟赛树里不要依赖位置类逻辑。
4. **阈值与坐标需按联盟赛规则确定**：回家血量/弹量阈值、前哨站坐标、巡逻点等需与赛事规则对齐。
5. **黑板键名必须与 `intput_blackboard.hpp` 一致**，否则端口读不到数据。

## 8. 落地步骤（Checklist）

- [ ] 确认联盟赛规则：回家阈值、战术优先级、目标坐标、巡逻点。
- [ ] 填写 `rmul_2027.xml`（照骨架替换阈值/坐标，仅用已注册节点）。
- [ ] `param.yaml`：`is_decision: true` + `tree_xml_file` 指向新树。
- [ ] 启动节点，观察日志确认树加载成功、`goal` 正常发布。
- [ ] （可选）Groot 加载 `generated_models.xml` 可视化验证树结构。