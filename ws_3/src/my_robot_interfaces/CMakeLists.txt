cmake_minimum_required(VERSION 3.28.3)
project(my_robot_interfaces)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-strict-prototypes)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
        "action/CountUntil.action"
)

# =============================================================================
# 【警告修复模块】消除 rosidl 自动生成C代码产生的 [-Wstrict-prototypes] 告警
set(typesupport_c_targets
        ${PROJECT_NAME}__rosidl_typesupport_introspection_c
        ${PROJECT_NAME}__rosidl_typesupport_c
)
foreach (target ${typesupport_c_targets})
    if (TARGET ${target})
        target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:C>:-Wno-strict-prototypes>
        )
    endif ()
endforeach ()
# =============================================================================

ament_export_dependencies(rosidl_default_runtime)

ament_package()




