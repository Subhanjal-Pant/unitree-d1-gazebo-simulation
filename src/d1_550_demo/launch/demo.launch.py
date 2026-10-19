from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    moveit_config = MoveItConfigsBuilder(
        "d1_550", package_name="d1_550_moveit_config"
    ).to_moveit_configs()

    demo = Node(
        package="d1_550_demo",
        executable="pick_demo",
        output="screen",
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            {"use_sim_time": True},
        ],
    )
    return LaunchDescription([demo])