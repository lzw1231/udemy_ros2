cmake_minimum_required(VERSION 4.0)
project(lifecycle_cpp)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)


# 编译可执行文件
add_executable(number_publisher src/number_publisher.cpp)
ament_target_dependencies(
        number_publisher
        example_interfaces
        rclcpp
        rclcpp_lifecycle
)


install(TARGETS
        number_publisher
        DESTINATION lib/${PROJECT_NAME}
)

ament_package()
