# ROS2 Python启动文件，替代ROS1的xml/launch文件，支持逻辑处理、动态参数、延时控制等高级功能

import os

import ros_gz_sim
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command
from launch_ros.actions import Node


def generate_launch_description():
    # 定义各包路径
    robot_desc_pkg = get_package_share_directory("my_robot_description")
    bringup_pkg = get_package_share_directory("my_robot_bringup")
    ros_gz_sim_pkg = get_package_share_directory("ros_gz_sim")

    # 定义各文件路径
    world_file = os.path.join(bringup_pkg, "worlds", "test_world.sdf")
    urdf_path = os.path.join(robot_desc_pkg, "urdf", "my_robot.urdf.xacro")
    rviz_cfg = os.path.join(robot_desc_pkg, "rviz", "urdf_config.rviz")
    bridge_yaml = os.path.join(bringup_pkg, "config", "ros_gz_bridge.yaml")

    # 1. 机器人状态发布器：编译xacro生成完整URDF存入参数服务器，发布robot_description话题与TF坐标变换
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": Command(["xacro ", urdf_path])}],
        output="screen"
    )

    # 2. 启动Gazebo仿真环境，加载空白世界，-r 参数表示仿真启动后直接运行
    gz_sim_launcher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim_pkg, "launch/gz_sim.launch.py")),
        launch_arguments={"gz_args": [world_file, " -r"]}.items()
    )

    # 3. 订阅ROS2 /robot_description话题获取机器人URDF模型，在gz中创建机器人物理实体
    spawn_robot_node = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-topic", "robot_description"]
    )

    # 4. ROS2与Gazebo双向话题桥，读取yaml配置文件转发传感器、控制类消息
    ros_gz_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        parameters=[{"config_file": bridge_yaml}]
    )

    # 5. 启动RViz可视化工具，加载预设配置文件，展示机器人模型、TF与仿真传感器数据
    rviz2_node = Node(
        package="rviz2",
        executable="rviz2",
        # arguments=["-d", rviz_cfg],
        output="screen"
    )
    # 6. 启动关节控制器
    joint_state_gui_node = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        output="screen"
    )

    # 6. 整合所有节点并依次启动
    return LaunchDescription([
        robot_state_publisher_node,
        gz_sim_launcher,
        spawn_robot_node,
        ros_gz_bridge_node,
        rviz2_node,
        # joint_state_gui_node
    ])
