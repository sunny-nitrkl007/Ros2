from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    pkg_share = get_package_share_directory('rsg_v1_three_node_cross_pubsub_matrix')
    params = os.path.join(pkg_share, 'config', 'params.yaml')
    return LaunchDescription([
        Node(package='rsg_v1_three_node_cross_pubsub_matrix', executable='node_a', name='node_a', namespace='/matrix_demo', output='screen', parameters=[params], arguments=['--ros-args', '--log-level', 'info']),
        Node(package='rsg_v1_three_node_cross_pubsub_matrix', executable='node_b', name='node_b', namespace='/matrix_demo', output='screen', parameters=[params], arguments=['--ros-args', '--log-level', 'info']),
        Node(package='rsg_v1_three_node_cross_pubsub_matrix', executable='node_c', name='node_c', namespace='/matrix_demo', output='screen', parameters=[params], arguments=['--ros-args', '--log-level', 'info']),
    ])
