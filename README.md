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

The first thing I needed was a docker image so this was added.  This directory is essentially the same as used by the RasPi robot project and has been used with several projects so is pretty stable now.

When I first started to implement drivers etc., I realised that I needed to use more than just one file, so I implemented a CMAKE fix to allow a project to have many files.  This takes the form of a patch to the file `/microros_esp32_extensions/main/CMakeLists.txt` and then adding a new file `CMakeLists.txt` to the application.

## Getting started

Follow [these instructions](docker/README.md) to setup the docker and workspace used to build the project.
