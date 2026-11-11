# rmuc2027 决策树结构

## 完整树（子树已展开接入主树）

```
main_tree
└── Sequence
    ├── IsGameStart                        is_game_start
    └── ReactiveFallback
        ├── Sequence  ────────── [回家序列·优先级最高]
        │   ├── CheckValue                弹量 ≤ 10
        │   ├── CheckValue                血量 ≤ 60
        │   ├── Nav2Pose                  目标 (0.0, 0.0, 1.0)
        │   └── Sleep                     4000 ms
        └── Sequence  ────────── [健康序列]
            ├── CheckValue                弹量 ≥ 70
            ├── CheckValue                血量 ≥ 350
            └── ReactiveFallback  ── [备选任务·按优先级]
                ├── SubTree: hit_enemy_outpost_subtree  ── 攻敌方哨站
                │   └── Sequence
                │       ├── CheckValue    哨站血 ≤ 40
                │       ├── Nav2Pose      目标 (2.65, 4.36, 1.0)
                │       └── Sleep         2000 ms
                ├── SubTree: protect_hero_subtree  ── 护英雄
                │   └── Sequence
                │       ├── CheckValue    时间 ≤ 420
                │       ├── CheckValue    时间 ≥ 400
                │       ├── Nav2Pose      目标 (2.65, 4.36, 1.0)
                │       └── Sleep         2000 ms
                └── SubTree: patrol_subtree  ── 4点巡逻
                    └── Repeat            -1（无限循环）
                        └── Sequence
                            ├── Timeout    24000 ms
                            │   └── Nav2Pose  目标 (2.18, 3.2, 1.0)
                            ├── Sleep      1000 ms
                            ├── Timeout    25000 ms
                            │   └── Nav2Pose  目标 (2.15, -0.73, 1.0)
                            ├── Sleep      1000 ms
                            ├── Timeout    25000 ms
                            │   └── Nav2Pose  目标 (8.56, 3.07, 1.0)
                            ├── Sleep      1000 ms
                            ├── Timeout    25000 ms
                            │   └── Nav2Pose  目标 (10.6, -0.54, 1.0)
                            └── Sleep      1000 ms
```

## 节点图例

| 节点 | 类型 | 作用 |
|------|------|------|
| `Sequence` | 控制节点 | 顺序执行，任一失败即失败 |
| `ReactiveFallback` | 控制节点 | 从左到右找第一个成功的分支 |
| `Repeat` | 装饰节点 | 循环执行子节点，`-1` 为无限循环 |
| `Timeout` | 装饰节点 | 子节点超时未完成则 FAILURE |
| `Sleep` | 动作节点 | 同步延时 |
| `CheckValue` | 条件节点 | 黑板 int 值比较 |
| `IsGameStart` | 条件节点 | 判断比赛是否开始 |
| `Nav2Pose` | 动作节点 | 发布导航目标点 |
| `SubTree` | 子树 | 引用另一棵行为树 |

## 未接入的子树

`protect_fort_subtree`（守哨站）在主树中被注释，未接入：

```
protect_fort_subtree
└── Sequence
    ├── IsFortOccupied                   ours_fort_occ_state
    ├── Nav2Pose                        目标 (2.65, 4.36, 1.0)
    └── Sleep                           2000 ms
```

## 注意

- `CheckValue` 当前**未注册**，本树加载会失败，需在 `rm_decision.cpp` 注册：
  `factory_.registerNodeType<bt::CheckValue>("CheckValue");`
- 回家序列用两个 `CheckValue` 的 **AND** 语义，与已注册 `IsGoHome` 的 **OR** 语义不符。
- 回家/攻哨站/护英雄的 `Nav2Pose` 无 `Timeout` 保护。