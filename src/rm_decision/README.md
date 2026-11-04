# ma 2027 sentry decision

robomaster2027 ma 哨兵决策

## 编写规范(怎么增加新的功能)

### 1. include/rm_decision 下是对外 public的hpp ,如果是private的hpp放在src下

将需要的接口，以及对外的接口放在include/rm_decision下，其他的放在src下


### 2. rm_decision.hpp/.cpp 是总入口

### 3. 增加新的节点

src下根据节点类型分类放置文件

根据bt的接口 编写对应的节点(看其他的同类型节点编写)

rm_decision.hpp/.cpp中setting_tree_custom_node增加注册对应的节点

增加节点时
a.先考虑是否值得增加，
b.可否有现有节点进行逻辑组装
c.优先考虑封装原子逻辑功能,复杂逻辑由bt子树构建，如果是特殊算法逻辑则考虑封装
d.优先编写常用的逻辑---->节点

### 4.外部输入接口

decision模块所需要的数据由 src/rm_decision/include/rm_decision/api.hpp 中定义

rm_decision.hpp/.tree_tick()调用输入

目前(2026.11.4)是  
    serial/socket---->game data----                    ---->serial/socket-->下位机
                                    |-->bt-----ros2-->|
    nav------------->nav data------                    ---->ros2 topic----->nav导航模块

***不要随意的进行数据的输入输出，保证模块的出入口单一，以及接口统一***

### 5. 输出外部的接口

rm_decision.hpp/.cpp中get_data()调用输出，数据定义在src/rm_decision/include/rm_decision/api.hpp 和4一致

### 6. bt行为树的输入输出

src/rm_decision/include/rm_decision/action中的intput_blackboard.hpp output_blackboard.hpp是数据输入输出bt黑板的接口

***如果外部输入和输出外部的数据增加或减少，要在intput_blackboard.hpp output_blackboard.hpp中对应更改***

除了nav game data ,语义上不应该有其他的输入输出


🟦 蓝色：节点返回 SUCCESS（执行成功）。

🟥 红色：节点返回 FAILURE（执行失败）。

🟩 绿色：节点正在执行 RUNNING（比如你的 Nav2Pose 正在导航中）。

⬜ 灰色：节点还未被执行（闲置状态）。

🟨 黄色：节点被强制终止 HALTED。



## 决策逻辑设计

### 1. 安全层

1. hp, projectile_allowance---->go home

### 2. 保底层

1. 巡逻

2. 追击

3. 补给

### 3. 动态规划层

1. 考虑敌我双方的位置，状态

2. 考虑基地，前哨站