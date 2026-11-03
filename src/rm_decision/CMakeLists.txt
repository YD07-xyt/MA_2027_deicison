cmake_minimum_required(VERSION 3.16)
project(rm_decision VERSION 0.10)


set(LIB_NAMES behaviortree_cpp spdlog)
set(LIB_3RD "")

foreach(LIB_NAME ${LIB_NAMES})
    find_package(${LIB_NAME} REQUIRED)
    list(APPEND LIB_3RD "${LIB_NAME}::${LIB_NAME}")
endforeach()

find_package(Eigen3 REQUIRED)
list(APPEND LIB_3RD Eigen3::Eigen)

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
target_link_libraries(${PROJECT_NAME} PUBLIC  ${LIB_3RD})
target_link_libraries(${PROJECT_NAME} PUBLIC  tools)