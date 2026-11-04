
/**
example:

<!-- key 从黑板取：用 {变量名} -->
<CheckValue key="{hp}"      operator="gt" value="0"/>   <!-- hp >  0  -->
<CheckValue key="{hp}"      operator="le" value="20"/>  <!-- hp <= 20 -->
<CheckValue key="{ammo}"    operator="eq" value="0"/>   <!-- ammo == 0 -->
<CheckValue key="{mode}"    operator="ne" value="1"/>   <!-- mode != 1 -->
<CheckValue key="{temp}"    operator="ge" value="80"/>  <!-- temp >= 80 -->
<CheckValue key="{battery}" operator="lt" value="15"/>  <!-- battery < 15 -->

*/

#pragma once
#include "tools/logger.hpp"
#include "rm_decision/log.hpp"
#include <behaviortree_cpp/basic_types.h>
#include <behaviortree_cpp/condition_node.h>
#include <string>

// ===== 枚举 =====
enum class Op { EQ, NE, GT, GE, LT, LE };

// ===== 双向转换器（必须在节点类定义之前）=====
namespace BT {

template <>
inline Op convertFromString<Op>(StringView s) {
    // 首选：字母别名（XML 无转义，可读）
    if (s == "eq") return Op::EQ;
    if (s == "ne") return Op::NE;
    if (s == "gt") return Op::GT;
    if (s == "ge") return Op::GE;
    if (s == "lt") return Op::LT;
    if (s == "le") return Op::LE;
    // 兼容：旧 XML 里的符号形式（需 XML 转义）
    if (s == "="  || s == "==") return Op::EQ;
    if (s == "!=")              return Op::NE;
    if (s == ">")               return Op::GT;
    if (s == ">=")              return Op::GE;
    if (s == "<")               return Op::LT;
    if (s == "<=")              return Op::LE;

    throw RuntimeError(
        "invalid operator '" + std::string(s) +
        "', expected: eq|ne|gt|ge|lt|le (或 = != > >= < <=)");
}

template <>
inline std::string toStr<Op>(const Op& op) {
    switch (op) {
        case Op::EQ: return "eq";
        case Op::NE: return "ne";
        case Op::GT: return "gt";
        case Op::GE: return "ge";
        case Op::LT: return "lt";
        case Op::LE: return "le";
    }
    return "?";
}

} // namespace BT


namespace bt {

class CheckValue : public BT::ConditionNode {
public:
    CheckValue(const std::string& name, const BT::NodeConfig& config)
        : BT::ConditionNode(name, config) {}

    static BT::PortsList providedPorts() {
        return {
            BT::InputPort<int>("key",      "被检查的黑板 int"),
            BT::InputPort<Op> ("operator", "对比运算符: eq|ne|gt|ge|lt|le (兼容 = != > >= < <=)"),
            BT::InputPort<int>("value",    "对比值"),
        };
    }

    BT::NodeStatus tick() override {
        auto key      = getInput<int>("key");
        auto expected = getInput<int>("value");
        auto op       = getInput<Op> ("operator");

        if (!key)      { logger::warn(logger, "CheckValue: input 'key' failed");      return BT::NodeStatus::FAILURE; }
        if (!expected) { logger::warn(logger, "CheckValue: input 'value' failed");    return BT::NodeStatus::FAILURE; }
        if (!op)       { logger::warn(logger, "CheckValue: input 'operator' invalid"); return BT::NodeStatus::FAILURE; }

        return compare(*key, *expected, *op)
                   ? BT::NodeStatus::SUCCESS
                   : BT::NodeStatus::FAILURE;
    }

private:
    static bool compare(int k, int v, Op op) {
        switch (op) {
            case Op::EQ: return k == v;
            case Op::NE: return k != v;
            case Op::GT: return k >  v;
            case Op::GE: return k >= v;
            case Op::LT: return k <  v;
            case Op::LE: return k <= v;
        }
        return false;
    }
};

} // namespace bt