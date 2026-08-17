from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    ld = LaunchDescription()

    # 创建组件容器
    container = ComposableNodeContainer(
        name="my_container",  # 容器进程的名字（ros2 node list看不到这个，这是进程名）
        namespace="",  # 命名空间，空=根命名空间
        package="rclcpp_components",  # 包名：容器程序来自rclcpp_components
        executable="component_container",  # 可执行文件：官方通用组件容器
        composable_node_descriptions=[  # 列表：要加载到这个容器内部的所有组件
            ComposableNode(
                package="components_cpp",  # 组件所在功能包
                plugin="my_namespace::NumberPublisher",  # C++类的完整限定名，非常关键！
                name="number_publisher_1",  # ROS节点名称，ros2 node list能看到
            ),
            ComposableNode(
                package="components_cpp",
                plugin="my_namespace::NumberPublisher",
                name="number_publisher_2",
            ),
        ]
    )
    ld.add_action(container)
    return ld
