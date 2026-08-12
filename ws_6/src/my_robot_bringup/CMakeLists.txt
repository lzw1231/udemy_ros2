cmake_minimum_required(VERSION 4.0)
project(my_robot_bringup)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF CACHE INTERNAL "Disable compile_commands.json")

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# find dependencies
find_package(ament_cmake REQUIRED)

install(DIRECTORY
        launch
        DESTINATION share/${PROJECT_NAME})

ament_package()
