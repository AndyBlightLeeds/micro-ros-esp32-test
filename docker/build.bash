#!/bin/bash
set -e

# Copy code over.
cp -rf /home/build/code/multi_file_demo/ /home/build/ws/firmware/freertos_apps/apps

# Build the new code.
cd /home/build/ws
. ./install/local_setup.bash
ros2 run micro_ros_setup build_firmware.sh
