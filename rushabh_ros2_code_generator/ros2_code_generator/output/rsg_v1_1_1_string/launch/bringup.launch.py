from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    pkg_share = get_package_share_directory('rsg_v1_1_1_string')
    params = os.path.join(pkg_share, 'config', 'params.yaml')
    return LaunchDescription([
        Node(package='rsg_v1_1_1_string', executable='sender', name='sender', namespace='/robot1', output='screen', parameters=[params], arguments=['--ros-args', '--log-level', 'info']),
        Node(package='rsg_v1_1_1_string', executable='receiver', name='receiver', namespace='/robot1', output='screen', parameters=[params], arguments=['--ros-args', '--log-level', 'info']),
    ])
