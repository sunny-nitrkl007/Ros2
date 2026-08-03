from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch_ros.actions import Node


def generate_launch_description():
    job_manager = Node(
        package='lps_job_manager',
        executable='lps_job_manager_node',
        name='lps_job_manager',
        output='screen',
    )
    weigh_app = Node(
        package='lps_weigh_app',
        executable='lps_weigh_app_node',
        name='lps_weigh_app',
        output='screen',
    )
    return LaunchDescription([job_manager, weigh_app])
