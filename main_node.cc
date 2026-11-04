#include "ros2/node.h"
#include <rclcpp/logging.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/utilities.hpp>
#include <spdlog/spdlog.h>
#include <string>
int main(int argc, char *argv[]) {
  rclcpp::init(argc,argv);
  auto node=rclcpp::Node::make_shared("ma_decision_node");
  node->declare_parameter<std::string>("ma_decision_config");
  std::string yaml_path = node->get_parameter("ma_decision_config").as_string();

  ros2::Ros2config config;
   if (!load_config(yaml_path, config)) {
    return -1;
   }
  ros2::SerialNode serial_node(config,node);
  
  rclcpp::spin(node);

  rclcpp::shutdown();
  return 0;

}
