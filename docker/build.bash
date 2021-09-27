#!/bin/bash
set -e

# Copy code over.
rm -rf ~/ws/firmware/freertos_apps/apps/publishers
cp -rf ~/code/publishers/ ~/ws/firmware/freertos_apps/apps
rm -rf ~/ws/firmware/freertos_apps/apps/subscribers
cp -rf ~/code/subscribers/ ~/ws/firmware/freertos_apps/apps

# Build the new code.
cd ~/ws
. ./install/local_setup.bash
ros2 run micro_ros_setup build_firmware.sh

echo
echo "Flash using:"
echo ". /install/local_setup.bash"
echo "ros2 run micro_ros_setup flash_firmware.sh "
echo
