#!/bin/bash
set -e

# Copy code over.
cp -rf ~/code/publishers/ ~/ws/firmware/freertos_apps/apps

# Build the new code.
cd ~/ws
. ./install/local_setup.bash
ros2 run micro_ros_setup build_firmware.sh
