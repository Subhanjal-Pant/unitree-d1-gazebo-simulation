import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    desc_share = get_package_share_directory('d1_550_description')
    bringup_share = get_package_share_directory('d1_550_bringup')

    urdf_path = os.path.join(desc_share, 'urdf', 'd1_550_description.urdf')
    with open(urdf_path, 'r') as f:
        robot_description = f.read()

    rviz_config = os.path.join(bringup_share, 'rviz', 'display.rviz')
     
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
    )
    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
    )
    rviz_launch_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', rviz_config],
    )

    return LaunchDescription([
       robot_state_publisher_node,
       joint_state_publisher_gui_node,
       rviz_launch_node
    ])