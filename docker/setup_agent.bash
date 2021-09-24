#!/bin/bash
# This script is designed to be run from inside the ESP32 docker.
set -e

# Create a workspace and download the micro-ROS tools
mkdir -p ~/ws/src
cd ~/ws/src
if [ ! -e ~/ws/src/micro_ros_setup ]
then
	echo "Please run 'setup_publishers.bash' first."
	exit 1
fi

# Setup and build the agent.
cd ~/ws
sudo apt update
rosdep update
. /opt/ros/foxy/setup.bash
ros2 run micro_ros_setup create_agent_ws.sh
ros2 run micro_ros_setup build_agent.sh

echo "To run the agent:"
echo ". install/local_setup.bash"
echo "ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888"
echo
echo "To rebuild the agent:"
echo ". install/local_setup.bash"
echo "ros2 run micro_ros_setup build_agent.sh"
echo ". install/local_setup.bash"
echo
