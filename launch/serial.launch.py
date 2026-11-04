import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    package_name = 'ma_decision'
    config_dir = os.path.join(get_package_share_directory(package_name), 'config')
    default_param_file = os.path.join(config_dir, 'param.yaml')

    # 声明启动参数 config_file
    config_file_arg = DeclareLaunchArgument(
        'config_file',
        default_value=default_param_file,
        description='Path to ma_decision yaml config file'
    )

    ma_decision_node = Node(
        package=package_name,
        executable='ma_decision_node',
        name='ma_decision_node',
        parameters=[
            # 把启动参数 config_file 赋值给 ROS2 参数 ma_decision_config
            {"ma_decision_config": LaunchConfiguration('config_file')}
        ],
        output='screen',
    )

    return LaunchDescription([
        config_file_arg,
        ma_decision_node
    ])
