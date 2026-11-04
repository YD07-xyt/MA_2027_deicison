cmake_minimum_required(VERSION 3.16)

# 纯头文件工具库（logger、expected 等），不再包含源文件
add_library(tools INTERFACE)
target_include_directories(tools INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
