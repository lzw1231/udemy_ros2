from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    node_a = Node(
        package="lifecycle_py",
        executable="move_robot_server",
        name="move_robot_server_a",
        parameters=[{"robot_name": "A"}],
    )

    node_b = Node(
        package="lifecycle_py",
        executable="move_robot_server",
        name="move_robot_server_b",
        parameters=[{"robot_name": "B"}],
    )

    startup_node = Node(
        package="lifecycle_py",
        executable="move_robot_startup",
        parameters=[{"managed_node_names": ["move_robot_server_a", "move_robot_server_b"]}],
    )

    return LaunchDescription([node_a, node_b, startup_node])
