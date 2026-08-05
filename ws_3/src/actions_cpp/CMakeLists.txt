# 设置CMake最低版本要求，4.0及以上
cmake_minimum_required(VERSION 4.0)
# 定义项目名称 actions_cpp
project(actions_cpp)

# GCC / Clang 编译器开启严格警告选项，便于代码质量检查
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# 查找ROS2依赖包
find_package(ament_cmake REQUIRED)          # ament构建系统核心
find_package(rclcpp REQUIRED)               # ROS2 C++客户端库
find_package(rclcpp_action REQUIRED)        # ROS2 Action C++接口库
find_package(my_robot_interfaces REQUIRED)  # 自定义消息/Action接口包

# Action服务端：基础版本
add_executable(count_until_server src/count_until_server.cpp)
ament_target_dependencies(count_until_server rclcpp rclcpp_action my_robot_interfaces)

# Action服务端：支持目标排队版本
add_executable(count_until_server_queue_goals src/count_until_server_queue_goals.cpp)
ament_target_dependencies(count_until_server_queue_goals rclcpp rclcpp_action my_robot_interfaces)

# Action客户端
add_executable(count_until_client src/count_until_client.cpp)
ament_target_dependencies(count_until_client rclcpp rclcpp_action my_robot_interfaces)

# 安装可执行文件到ROS2标准目录 lib/<pkg_name>
install(TARGETS
        count_until_server
        count_until_client
        count_until_server_queue_goals
        DESTINATION lib/${PROJECT_NAME}/
)

# ament包声明，生成package相关配置文件
ament_package()