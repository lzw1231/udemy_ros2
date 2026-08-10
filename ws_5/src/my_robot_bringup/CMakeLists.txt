cmake_minimum_required(VERSION 4.0)   # 务必使用有效版本
project(my_robot_bringup)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)

# 收集 launch 目录下所有 .py 文件（假设 launch 中只有 .py 文件）
file(GLOB LAUNCH_FILES launch/*.py)

# 安装这些文件到 share/${PROJECT_NAME}/launch
install(
        FILES ${LAUNCH_FILES}
        DESTINATION share/${PROJECT_NAME}/launch
)

ament_package()