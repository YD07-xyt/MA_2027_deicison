#pragma once

#include "rm_decision/action/intput_blackboard.hpp"
#include "rm_decision/action/output_blackboard.hpp"
#include "rm_decision/api.hpp"
#include "rm_decision/config.hpp"
#include <behaviortree_cpp/actions/sleep_node.h>
#include <behaviortree_cpp/behavior_tree.h>
#include <behaviortree_cpp/blackboard.h>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/json_export.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>
#include <behaviortree_cpp/xml_parsing.h>

#include <memory>
#include <optional>
#include <utility>

namespace bt {
class RmDecision {
public:
    explicit RmDecision(bt::DecisionConfig decision_config);
    /**
    @brief tick the behavior tree once, and update the blackboard with the latest game data and navigation data.
    */
    auto tree_tick(
        Game2Decision& game_data,
        Nav2Decision& nav_data,
        std::optional<std::chrono::system_clock::duration> timeout
    ) -> void;
    /**
    @brief get the latest decision data from the blackboard.
    @return a pair of optional decision data for game and navigation.
    */
    auto get_data() -> std::pair<std::optional<Decision2Game>, std::optional<Decision2Nav>>;

private:
    /**
    @brief register custom nodes and enums to the behavior tree factory.
    */
    void setting_tree_custom_node();

private:
    bt::DecisionConfig config_;
    std::shared_ptr<Intputblackboard> intput_blackboard_;
    std::shared_ptr<Outputblackboard> output_blackboard_;

private:
    BT::Tree tree_;
    BT::Blackboard::Ptr tree_blackboard_;
    std::shared_ptr<BT::Groot2Publisher> groot2publisher_ptr_;
    BT::BehaviorTreeFactory factory_;
};
} // namespace bt
