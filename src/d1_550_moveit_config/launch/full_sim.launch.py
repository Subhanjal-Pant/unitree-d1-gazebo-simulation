import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    bringup_share = get_package_share_directory('d1_550_bringup')
    moveit_share = get_package_share_directory('d1_550_moveit_config')

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(bringup_share, 'launch', 'gazebo.launch.py')
        )
    )

    moveit = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(moveit_share, 'launch', 'moveit_gazebo.launch.py')
        )
    )

    # Start MoveIt after Gazebo and the controllers have had time to come up
    return LaunchDescription([
        gazebo,
        TimerAction(period=10.0, actions=[moveit]),
    ])