# micro-ros-esp32-test

Minimal repo to test and debug micro-ROS features.

The main idea for this repo is to be able to generate pull requests that contain enhancements that I have develop on other projects.  Each pull request will be developed on a separate branch.

## Objectives

1. Develop ESP32 using a docker.
1. To allow bigger projects to be developed using the CMAKE file changes used on the RasPi robot project.
1. Fix limit of only one service.
1. Fix limit of five publishers.

### Background

The problems listed in the objectives above were discovered whilst developing my [ESP32 based RasPi robot](https://github.com/AndyBlightLeeds/raspi-robot-esp32).  The robot is fairly simple but needs to support a number of ROS messages, and I hit limits with the number of publishers and services.

When you start implementing drivers etc., you need to use more than just one file, so I implemented a CMAKE fix to allow a project to have many files.
