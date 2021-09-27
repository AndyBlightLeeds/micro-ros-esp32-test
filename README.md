# micro-ros-esp32-test

Minimal repo to test and debug micro-ROS features.

The main idea for this repo is to be able to generate pull requests that contain enhancements that I have develop on other projects.  Each pull request will be developed on a separate branch.

## Objectives

1. Develop ESP32 using a docker.
1. To allow bigger projects to be developed using the CMAKE file changes used on the RasPi robot project.
1. Fix limit of five publishers.
1. Fix limit of only one service.
1. Allow autonomous working.

### Background

The problems listed in the objectives above were discovered whilst developing my [ESP32 based RasPi robot](https://github.com/AndyBlightLeeds/raspi-robot-esp32).  The robot is fairly simple but needs to support a number of ROS messages, and I hit limits with the number of publishers and services.

The first thing I needed was a docker image so this was added.  This directory is essentially the same as used by the RasPi robot project and has been used with several projects so is pretty stable now.

When I first started to implement drivers etc., I realised that I needed to use more than just one file, so I implemented a CMAKE fix to allow a project to have many files.  This takes the form of a patch to the file `/microros_esp32_extensions/main/CMakeLists.txt` and then adding a new file `CMakeLists.txt` to the application.

## Develop using a docker

Follow [these instructions](docker/README.md) to setup the docker and workspace used to build the project.

## Allow sub-directories

This requires some patching. See branch `pr-allow-sub-directories`.

## Fix limit of 5 subscribers

After spending a day or so of pure frustration, I eventually found the problem.  The test code can be found on the branch `multi-subscribers-test`.

Changing the file `app-colcon.meta` and building using `ros2 run micro_ros_setup build_firmware.sh` does not cause any limits previously defined to be updated.  Only a full rebuild updates the limits.  By limits, I mean this for services `"-DRMW_UXRCE_MAX_SUBSCRIPTIONS=1",`.  When you change any values in the `app-colcon.meta` file, you need to rebuild as follows:

```c
cd ~/ws
. ./install/local_setup.bash
ros2 run micro_ros_setup configure_firmware.sh subscribers -t udp -i 192.168.54.2 -p 8888
ros2 run micro_ros_setup build_firmware.sh
```

NOTE: This full rebuild takes a while, about 6 minutes on my PC, so work out how many services, subscribers, etc. you need and set the values in `app-colcon.meta` accordingly.  The code can be updated later as you only need to make sure that you have allocated enough space.

## Fix limit of only one service



## Allow autonomous working

Requirement: The robot shall be capable of working without being connected to the micro-ROS client, e.g. when out of range.

The current start up code in `microros_esp32_extensions/main/main.c` insists that the Wi-Fi and micro-ROS client connections work before any other code is run.  To allow autonomous operation, is should be possible to boot the robot without Wi-Fi and allow it to connect when in range.

A separate task (or similar) could handle the Wi-Fi and micro-ROS client access. The task might do something like this:

1. Try to connect to the Wi-Fi.  Repeat at regular intervals (say once a second) until connection established.
2. When Wi-Fi is available, try to connect to the micro-ROS client. Repeat at regular intervals (say once a second) until connection established.
3. This is now in connected mode and all communications take place as expected.
4. If the connection to the micro-ROS client stops, goto 1.

One thing to note is that the `microros_esp32_extensions/main/main.c` file has a function `app_main` that has blocks of code to start the Wi-Fi or the serial port for micro-ROS.  Both can be disabled by not defining `RMW_UXRCE_TRANSPORT_UDP` __and__ `RMW_UXRCE_TRANSPORT_CUSTOM`.  The Wi-Fi connection code used in the function `wifi_init_sta` can be used as an example in the new task.
