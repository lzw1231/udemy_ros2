import os

# 获取功能包共享目录路径工具
from ament_index_python.packages import get_package_share_path
# 启动任务容器
from launch import LaunchDescription
# 执行终端命令指令
from launch.substitutions import Command
# 启动节点类
from launch_ros.actions import Node
# 封装命令输出为ROS参数
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    # 拼接机器人xacro模型文件完整路径
    urdf_path = os.path.join(
        get_package_share_path("my_robot_description"), "urdf", "my_robot.urdf.xacro"
    )

    # 拼接RViz配置文件完整路径
    rviz_config_path = os.path.join(
        get_package_share_path("my_robot_description"), "rviz", "urdf_config.rviz"
    )

    # 调用xacro解析模型文件，转为robot_description参数字符串
    robot_description = ParameterValue(Command(["xacro ", urdf_path]), value_type=str)

    # 机器人状态发布节点：发布TF、连杆坐标
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}],
    )

    # 关节状态GUI调节节点：可视化滑块拖动关节
    joint_state_publisher_gui_node = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui"
    )

    # RViz可视化节点，加载预设配置
    rviz2_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config_path]
    )

    # 打包所有节点并返回启动描述
    return LaunchDescription([
        robot_state_publisher_node,
        joint_state_publisher_gui_node,
        rviz2_node
    ])
