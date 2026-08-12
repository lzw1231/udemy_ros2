cmake_minimum_required(VERSION 4.0)

project(my_robot_bringup)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

find_package(ament_cmake REQUIRED)


install(DIRECTORY
        launch
        DESTINATION share/${PROJECT_NAME}
)


ament_package()