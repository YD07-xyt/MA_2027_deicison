cmake_minimum_required(VERSION 3.16)
project(rm_decision VERSION 0.10)

find_package(behaviortree_cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(Eigen3 REQUIRED)


find_package(ament_cmake REQUIRED)
set(ROS2_LIB_NAMES geometry_msgs rclcpp action_msgs nav2_msgs tf2_eigen nav2_common)
foreach(LIB_NAME ${ROS2_LIB_NAMES})
    find_package(${LIB_NAME} REQUIRED)
endforeach()


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME}  OBJECT 
    ${SOURCES}
)
# 公开头文件路径（外部可见）
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
# 私有头文件路径（仅编译本库时可见）
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

ament_target_dependencies(${PROJECT_NAME} PUBLIC ${ROS2_LIB_NAMES})
target_link_libraries(${PROJECT_NAME} PUBLIC  behaviortree_cpp::behaviortree_cpp spdlog::spdlog Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} PUBLIC  tools)