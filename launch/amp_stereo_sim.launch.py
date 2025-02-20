from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.actions import DeclareLaunchArgument
from launch.actions import ExecuteProcess, TimerAction

def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'namespace',
            default_value=['orbslam3']
        ),
        DeclareLaunchArgument(
            'vocabulary',
            default_value=PathJoinSubstitution([
                FindPackageShare('orbslam3_ros2'),
                'vocabulary',  # Assuming your vocab file is in the vocabulary directory
                'ORBvoc.txt'   # Replace with your actual vocabulary file name
            ]),
            description='Path to the ORB_SLAM3 vocabulary file'
        ),
        DeclareLaunchArgument(
            'pangolin',
            default_value="True",
            description='Use the viewer'
        ),
        DeclareLaunchArgument(
            'yaml_file',
            default_value='amp_stereosim.yaml',
            description='Name of the ORB_SLAM3 YAML configuration file'
        ),
        DeclareLaunchArgument(
            'namespace',
            default_value=['orbslam3']
        ),
        DeclareLaunchArgument(
            'frame_id',
            default_value=['orbslam3']
        ),
        DeclareLaunchArgument(
            'child_frame_id',
            default_value=['left_camera_link']
        ),
        DeclareLaunchArgument(
            'left_camera',
            default_value=['/stereo_left']
        ),
        DeclareLaunchArgument(
            'right_camera',
            default_value=['/stereo_right']
        ),

        Node(
            package='orbslam3_ros2',
            executable='stereo',
            name='stereo_orbslam3',
            output='screen',
            namespace=LaunchConfiguration('namespace'),
            arguments=[
                LaunchConfiguration('vocabulary'),
                PathJoinSubstitution([
                    FindPackageShare('orbslam3_ros2'),
                    'config',  
                    'stereo',
                    LaunchConfiguration('yaml_file')
                ]),
                'False',
                LaunchConfiguration('pangolin')
            ],
            remappings=[
                ('camera/left', LaunchConfiguration('left_camera')),
                ('camera/right', LaunchConfiguration('right_camera'))
            ],
            parameters = [{'frame_id':LaunchConfiguration('frame_id'), 'child_frame_id':LaunchConfiguration('child_frame_id')}]
        ),
    
    ])
