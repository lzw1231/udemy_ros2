from launch import LaunchDescription
from launch_ros.actions import LifecycleNode
from launch_ros.actions import Node


def generate_launch_description():
    """生成Launch描述：创建一个生命周期节点及其对应的状态管理器节点。"""
    ld = LaunchDescription()

    # 被管理的生命周期节点的名称（ROS节点名）
    managed_node_name = "abcdefg"

    # 创建生命周期节点（初始状态为 'unconfigured'）
    # 该节点由 lifecycle_cpp 包中的 number_publisher 可执行文件实现
    number_lifecycle_node = LifecycleNode(
        package="lifecycle_cpp",  # 功能包名称
        executable="number_publisher",  # 可执行文件名
        name=managed_node_name,  # 节点名称（用于标识）
        namespace="",  # 命名空间（空表示全局）
    )

    # 创建生命周期管理器节点
    # 该节点会通过参数获取被管理节点的名称，并自动执行 configure → activate 转换
    lifecycle_manager_node = Node(
        package="lifecycle_py",
        executable="lifecycle_node_manager",
        parameters=[{"managed_node_name": managed_node_name}],  # 传递目标节点名
    )

    # 将两个节点动作添加到启动描述中
    ld.add_action(number_lifecycle_node)
    ld.add_action(lifecycle_manager_node)

    return ld
