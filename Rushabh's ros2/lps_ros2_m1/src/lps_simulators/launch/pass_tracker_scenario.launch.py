from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(package='lps_job_manager', executable='lps_job_manager_node',
             name='lps_job_manager', output='screen'),
        Node(package='lps_simulators', executable='lps_pass_tracker_scenario_node',
             name='lps_pass_tracker_scenario', output='screen'),
    ])
